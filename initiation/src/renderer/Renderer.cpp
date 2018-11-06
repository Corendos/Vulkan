#include "renderer/Renderer.hpp"

#include <chrono>
#include <iostream>

#include "vulkan/ColorAttachment.hpp"
#include "vulkan/DepthAttachment.hpp"
#include "vulkan/Subpass.hpp"
#include "vulkan/SubpassDependency.hpp"
#include "vulkan/UniformBufferObject.hpp"
#include "vulkan/Commands.hpp"
#include "vulkan/BufferHelper.hpp"
#include "colors/Color.hpp"
#include "camera/CameraInfo.hpp"
#include "environment.hpp"

Renderer::Renderer() {
    mVertexShader = Shader(shaderPath + "vert.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
    mFragmentShader = Shader(shaderPath + "frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");

    mClearValues[0].color = {0.325f, 0.694f, 0.937f, 1.0f};
    mClearValues[1].depthStencil = {1.0f, 0};
}

void Renderer::create(VulkanContext& context) {
    if (mCreated) {
        return;
    }

    mContext = &context;

    mSwapChain.query(mContext->getWindow(),
                     mContext->getPhysicalDevice(),
                     mContext->getDevice(),
                     mContext->getSurface());
    mExtent = mSwapChain.getExtent();
    createRenderPass();
    createDepthResources();
    mSwapChain.create(mContext->getWindow(),
                      mContext->getPhysicalDevice(),
                      mContext->getDevice(),
                      mContext->getSurface(),
                      mContext->getQueueFamilyIndices(),
                      mDepthImageView,
                      mRenderPass);
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createDescriptorPool();
    createCameraUniformBuffers();
    createCameraDescriptorSets();
    mStaticObjectManager.create(*mContext, *this);
    createCommandBuffers();
    createSemaphores();
    createFences();

    mCreated = true;
    mRecreated = true;
}

void Renderer::recreate() {
    int width{0}, height{0};
    while(width == 0 || height == 0) {
        glfwGetFramebufferSize(mContext->getWindow(), &width, &height);
        glfwWaitEvents();
    }
    
    vkDeviceWaitIdle(mContext->getDevice());

    vkDestroyImageView(mContext->getDevice(), mDepthImageView, nullptr);
    mContext->getMemoryManager().freeImage(mDepthImage);
    vkFreeCommandBuffers(mContext->getDevice(), mContext->getCommandPool().getHandler(),
                         static_cast<uint32_t>(mCommandBuffers.size()),
                         mCommandBuffers.data());

    mPipeline.destroy(mContext->getDevice());
    mRenderPass.destroy(mContext->getDevice());
    mSwapChain.destroy(mContext->getDevice());

    mSwapChain.query(mContext->getWindow(), mContext->getPhysicalDevice(), mContext->getDevice(), mContext->getSurface());
    mExtent = mSwapChain.getExtent();
    createRenderPass();
    createDepthResources();
    mSwapChain.create(mContext->getWindow(), mContext->getPhysicalDevice(), mContext->getDevice(),
                      mContext->getSurface(), mContext->getQueueFamilyIndices(), mDepthImageView,
                      mRenderPass);
    createRenderPass();
    createGraphicsPipeline();
    createCommandBuffers();
    mCamera->setExtent(mSwapChain.getExtent());
}

void Renderer::destroy() {
    if (mCreated) {
        for (size_t i{0};i < mSwapChain.getImageCount();++i) {
            mContext->getMemoryManager().freeBuffer(mCameraUniformBuffers[i]);
        }
        mVertexShader.destroy(mContext->getDevice());
        mFragmentShader.destroy(mContext->getDevice());
        mContext->getMemoryManager().freeImage(mDepthImage);
        vkDestroyImageView(mContext->getDevice(), mDepthImageView, nullptr);
        vkFreeCommandBuffers(mContext->getDevice(), mContext->getCommandPool().getHandler(), static_cast<uint32_t>(mCommandBuffers.size()), mCommandBuffers.data());
        mPipeline.destroy(mContext->getDevice());
        mRenderPass.destroy(mContext->getDevice());
        mSwapChain.destroy(mContext->getDevice());
        mStaticObjectManager.destroy();
        vkDestroyDescriptorPool(mContext->getDevice(), mDescriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(mContext->getDevice(), mTextureDescriptorSetLayout, nullptr);
        vkDestroyDescriptorSetLayout(mContext->getDevice(), mCameraDescriptorSetLayout, nullptr);
        vkDestroySemaphore(mContext->getDevice(), mImageAvailableSemaphore, nullptr);
        vkDestroySemaphore(mContext->getDevice(), mRenderFinishedSemaphore, nullptr);
        for (size_t i{0};i < mFences.size();++i) {
            vkDestroyFence(mContext->getDevice(), mFences[i], nullptr);
        }
        mContext->getCommandPool().destroy(mContext->getDevice());
        mCreated = false;
    }
}

void Renderer::render() {
    if (mBypassRendering) return;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {mImageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mCommandBuffers[mNextImageIndex];

    VkSemaphore signalSemaphores[] = {mRenderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(mContext->getGraphicsQueue(), 1, &submitInfo, mFences[mNextImageIndex]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer");
    }
    mIsFenceSubmitted[mNextImageIndex] = true;

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {mSwapChain.getHandler()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &mNextImageIndex;
    presentInfo.pResults = nullptr;

    VkResult result = vkQueuePresentKHR(mContext->getGraphicsQueue(), &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreate();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("  Failed to present image");
    }
}

void Renderer::update() {
    mBypassRendering = false;
    VkResult result = vkAcquireNextImageKHR(
        mContext->getDevice(), mSwapChain.getHandler(), std::numeric_limits<uint64_t>::max(),
        mImageAvailableSemaphore, VK_NULL_HANDLE, &mNextImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate();
        mBypassRendering = true;
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swap chain image");
    }

    updateUniformBuffer(mNextImageIndex);
    updateCommandBuffer(mNextImageIndex);
}

void Renderer::setCamera(Camera& camera) {
    mCamera = &camera;
}

void Renderer::setLight(Light& light) {
    mLight = &light;
}

VkDescriptorPool Renderer::getDescriptorPool() const {
    return mDescriptorPool;
}

// TODO: rename this
VkDescriptorSetLayout Renderer::getDescriptorSetLayout() const {
    return mTextureDescriptorSetLayout;
}

StaticObjectsManager& Renderer::getStaticObjectManager() {
    return mStaticObjectManager;
}

void Renderer::createRenderPass() {
    ColorAttachment colorAttachment;
    colorAttachment.setFormat(mSwapChain.getFormat());
    colorAttachment.setSamples(VK_SAMPLE_COUNT_1_BIT);
    colorAttachment.setLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR);
    colorAttachment.setStoreOp(VK_ATTACHMENT_STORE_OP_STORE);
    colorAttachment.setStencilLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE);
    colorAttachment.setStencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE);
    colorAttachment.setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED);
    colorAttachment.setFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    colorAttachment.setReferenceIndex(0);

    DepthAttachment depthAttachment;
    depthAttachment.setFormat(findDepthFormat());
    depthAttachment.setSamples(VK_SAMPLE_COUNT_1_BIT);
    depthAttachment.setLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR);
    depthAttachment.setStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE);
    depthAttachment.setStencilLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE);
    depthAttachment.setStencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE);
    depthAttachment.setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED);
    depthAttachment.setFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    depthAttachment.setReferenceIndex(1);

    mRenderPass.addAttachment(colorAttachment.getDescription(), colorAttachment.getReference());
    mRenderPass.addAttachment(depthAttachment.getDescription(), depthAttachment.getReference());

    Subpass subpass;
    subpass.setBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    subpass.addAttachment(colorAttachment, depthAttachment);

    mRenderPass.addSubpass(subpass.getDescription());

    SubpassDependency dependency;

    dependency.setSourceSubpass(VK_SUBPASS_EXTERNAL);
    dependency.setDestinationSubpass(0);
    dependency.setSourceStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    dependency.setSourceAccessMask(0);
    dependency.setDestinationStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    dependency.setDestinationAccessMask(VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

    mRenderPass.addSubpassDependency(dependency.getDependency());
    
    mRenderPass.create(mContext->getDevice());
}

void Renderer::createGraphicsPipeline() {
    mVertexShader.create(mContext->getDevice());
    mFragmentShader.create(mContext->getDevice());

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {
        mTextureDescriptorSetLayout,
        mCameraDescriptorSetLayout
    };

    PipelineLayout layout;
    layout.setDescriptorSetLayouts(descriptorSetLayouts);
    layout.create(mContext->getDevice());

    mPipeline.setPipelineLayout(layout);
    mPipeline.addShader(mVertexShader);
    mPipeline.addShader(mFragmentShader);
    mPipeline.setRenderPass(mRenderPass);
    mPipeline.setExtent(mExtent);
    mPipeline.create(mContext->getDevice());
}

void Renderer::createDepthResources() {
    VkFormat format = findDepthFormat();

    mDepthImage = Image::create(
        *mContext,
        mExtent.width, mExtent.height,
        format, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    mDepthImageView = Image::createImageView(*mContext, mDepthImage,
                                             format,
                                             VK_IMAGE_ASPECT_DEPTH_BIT);
    
    Image::transitionImageLayout(*mContext, mDepthImage,
                                 format, VK_IMAGE_LAYOUT_UNDEFINED,
                                 VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void Renderer::createDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 10000;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = 10000;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 10000;

    if (vkCreateDescriptorPool(mContext->getDevice(), &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool");
    }
}

void Renderer::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding textureBinding{};
    textureBinding.binding = 0;
    textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureBinding.descriptorCount = 1;
    textureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo textureLayoutInfo{};
    textureLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    textureLayoutInfo.bindingCount = 1;
    textureLayoutInfo.pBindings = &textureBinding;

    if (vkCreateDescriptorSetLayout(mContext->getDevice(), &textureLayoutInfo, nullptr, &mTextureDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout");
    }

    VkDescriptorSetLayoutBinding cameraLayoutBinding{};
    cameraLayoutBinding.binding = 0;
    cameraLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    cameraLayoutBinding.descriptorCount = 1;
    cameraLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    cameraLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo cameraLayoutInfo{};
    cameraLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    cameraLayoutInfo.bindingCount = 1;
    cameraLayoutInfo.pBindings = &cameraLayoutBinding;

    if (vkCreateDescriptorSetLayout(mContext->getDevice(), &cameraLayoutInfo, nullptr, &mCameraDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout");
    }
}

void Renderer::createCommandBuffers() {
    mCommandBuffers.resize(mSwapChain.getImageCount());

    Commands::allocateBuffers(mContext->getDevice(), mContext->getCommandPool(), mCommandBuffers);
}

void Renderer::createCameraUniformBuffers() {
    mCameraUniformBuffers.resize(mSwapChain.getImageCount());
    VkDeviceSize bufferSize = sizeof(CameraInfo);

    for (size_t i{0};i < mCameraUniformBuffers.size();++i) {
        BufferHelper::createBuffer(mContext->getMemoryManager(),
                               mContext->getDevice(),
                               bufferSize,
                               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
                               mCameraUniformBuffers[i]);
    }
}

void Renderer::createCameraDescriptorSets() {
    mCameraDescriptorSets.resize(mSwapChain.getImageCount());

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = mDescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &mCameraDescriptorSetLayout;

    for (size_t i{0};i < mCameraDescriptorSets.size();++i) {
        if (vkAllocateDescriptorSets(mContext->getDevice(), &allocInfo, &mCameraDescriptorSets[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate descriptor sets");
        }
    }

    for (size_t i{0};i < mCameraDescriptorSets.size();++i) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = mCameraUniformBuffers[i];
        bufferInfo.range = sizeof(CameraInfo);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = mCameraDescriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(mContext->getDevice(), 1, &descriptorWrite, 0, nullptr);
    }
}

void Renderer::createSemaphores() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    if ((vkCreateSemaphore(mContext->getDevice(), &semaphoreInfo, nullptr, &mImageAvailableSemaphore) != VK_SUCCESS) ||
        (vkCreateSemaphore(mContext->getDevice(), &semaphoreInfo, nullptr, &mRenderFinishedSemaphore) != VK_SUCCESS)) {
        throw std::runtime_error("Failed to create semaphores");
    }
}

void Renderer::createFences() {
    mFences.resize(mSwapChain.getImageCount());
    mIsFenceSubmitted.resize(mSwapChain.getImageCount());
    VkFenceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    for (size_t i{0};i < mFences.size();++i) {
        if (vkCreateFence(mContext->getDevice(), &createInfo, nullptr, &mFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create fences");
        }
    }
}

void Renderer::updateCommandBuffer(uint32_t index) {
    if (mIsFenceSubmitted[index]) {
        if (vkGetFenceStatus(mContext->getDevice(), mFences[index]) == VK_NOT_READY) {
            std::cout << "Fence #" << index << " not ready" << std::endl;
            return;
        } else {
            vkResetFences(mContext->getDevice(), 1, &mFences[index]);
        }
    }

    Commands::begin(mCommandBuffers[index], VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = mRenderPass.getHandler();
    renderPassInfo.framebuffer = mSwapChain.getFramebuffers()[index];

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = mSwapChain.getExtent();

    renderPassInfo.clearValueCount = static_cast<uint32_t>(mClearValues.size());
    renderPassInfo.pClearValues = mClearValues.data();

    vkCmdBeginRenderPass(mCommandBuffers[index], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(mCommandBuffers[index], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline.getHandler());
    
    VkBuffer vertexBuffers[] = {mStaticObjectManager.getVertexBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(mCommandBuffers[index], 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(mCommandBuffers[index], mStaticObjectManager.getIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);
    
    vkCmdBindDescriptorSets(mCommandBuffers[index],
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            mPipeline.getLayout().getHandler(),
                            1, 1, &mCameraDescriptorSets[index],
                            0, nullptr);

    for (size_t i{0};i < mStaticObjectManager.getObjectInfos().size();++i) {
        VkDescriptorSet descriptors[] = {mStaticObjectManager.getDescriptor(i)};
        vkCmdBindDescriptorSets(mCommandBuffers[index],
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                mPipeline.getLayout().getHandler(),
                                0, 1, descriptors,
                                0, nullptr);
        
        vkCmdDrawIndexed(mCommandBuffers[index],
                         mStaticObjectManager.getObjectInfo(i).indiceCount,
                         1, mStaticObjectManager.getObjectInfo(i).indicesOffset, 0, 0);
    }        

    vkCmdEndRenderPass(mCommandBuffers[index]);

    if (vkEndCommandBuffer(mCommandBuffers[index]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffers");
    }
}

void Renderer::updateUniformBuffer(uint32_t index) {
    CameraInfo cameraInfo;
    cameraInfo.proj = mCamera->getProj();
    cameraInfo.view = mCamera->getView();
    cameraInfo.position = glm::vec4(mCamera->getPosition(), 1.0);
    cameraInfo.light = glm::vec4(mLight->position, 1.0);

    void* data;
    mContext->getMemoryManager().mapMemory(mCameraUniformBuffers[index], sizeof(CameraInfo), &data);
    memcpy(data, &cameraInfo, sizeof(CameraInfo));
    mContext->getMemoryManager().unmapMemory(mCameraUniformBuffers[index]);
}

VkFormat Renderer::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(mContext->getPhysicalDevice(), format, &properties);

        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
            return format;
        }

        throw std::runtime_error("failed to find supported format");
    }
}

VkFormat Renderer::findDepthFormat() {
    return findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}