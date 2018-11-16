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
#include "vulkan/ImageHelper.hpp"
#include "colors/Color.hpp"
#include "renderer/RenderInfo.hpp"
#include "environment.hpp"

Renderer::Renderer() {
    mVertexShader = Shader(shaderPath + "vert.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
    mFragmentShader = Shader(shaderPath + "frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");

    mClearValues[0].color = {0.325f, 0.694f, 0.937f, 1.0f};
    mClearValues[1].color = {0.0f, 0.0f, 0.0f, 0.0f};
    mClearValues[2].depthStencil = {1.0f, 0};
}

void Renderer::create(VulkanContext& context, TextureManager& textureManager) {
    if (mCreated) {
        return;
    }

    mContext = &context;
    mTextureManager = &textureManager;
    mObjectManager.create(*mContext);
    int size = 2;
    float space = 1.1f;
    for (int i = 0;i < size*size*size;++i) {
        int zInt = i / (size * size);
        int yInt = (i % (size * size)) / size;
        int xInt = (i % (size * size)) % size;
        float z = space * (float)zInt - space * (float)(size - 1) / 2.0f;
        float y = space * (float)yInt - space * (float)(size - 1) / 2.0f;
        float x = space * (float)xInt - space * (float)(size - 1) / 2.0f;
        Object o = Object::temp({x, y, z});
        o.setTexture(mTextureManager->getTexture("dirt"));
        mObjects.push_back(std::make_unique<Object>(std::move(o)));
    }
    for (size_t i{0};i < mObjects.size();++i) {
        mObjectManager.addObject(*mObjects[i]);
    }

    mObjectManager.updateBuffers();

    mSwapChain.query(mContext->getWindow(),
                     mContext->getPhysicalDevice(),
                     mContext->getDevice(),
                     mContext->getSurface());
    mExtent = mSwapChain.getExtent();
    createRenderPass();
    mSwapChain.create(mContext->getWindow(),
                      mContext->getPhysicalDevice(),
                      mContext->getDevice(),
                      mContext->getSurface(),
                      mContext->getQueueFamilyIndices(),
                      mRenderPass);
    createFramebuffers();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createDescriptorPool();
    createCameraUniformBuffers();
    createCameraDescriptorSets();
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

    for (auto& framebufferAttachment : mFramebufferAttachments) {
        framebufferAttachment.normal.image.destroy(*mContext);
        framebufferAttachment.normal.imageView.destroy(mContext->getDevice());
        framebufferAttachment.depth.image.destroy(*mContext);
        framebufferAttachment.depth.imageView.destroy(mContext->getDevice());
    }

    for (auto& framebuffer : mFrameBuffers) {
        framebuffer.destroy(mContext->getDevice());
    }

    vkFreeCommandBuffers(mContext->getDevice(), mContext->getGraphicsCommandPool().getHandler(),
                         static_cast<uint32_t>(mCommandBuffers.size()),
                         mCommandBuffers.data());

    mPipeline.destroy(mContext->getDevice());
    mRenderPass.destroy(mContext->getDevice());
    mSwapChain.destroy(mContext->getDevice());

    mSwapChain.query(mContext->getWindow(), mContext->getPhysicalDevice(), mContext->getDevice(), mContext->getSurface());
    mExtent = mSwapChain.getExtent();
    createRenderPass();
    mSwapChain.create(mContext->getWindow(), mContext->getPhysicalDevice(), mContext->getDevice(),
                      mContext->getSurface(), mContext->getQueueFamilyIndices(),
                      mRenderPass);
    createFramebuffers();
    createGraphicsPipeline();
    createCommandBuffers();
    mCamera->setExtent(mSwapChain.getExtent());
    for (size_t i{0};i < mCommandBufferNeedUpdate.size();++i) {
        mCommandBufferNeedUpdate[i] = true;
    }
}

void Renderer::destroy() {
    if (mCreated) {
        for (size_t i{0};i < mSwapChain.getImageCount();++i) {
            mContext->getMemoryManager().freeBuffer(mCameraUniformBuffers[i]);
        }

        for (auto& framebufferAttachment : mFramebufferAttachments) {
            framebufferAttachment.normal.image.destroy(*mContext);
            framebufferAttachment.normal.imageView.destroy(mContext->getDevice());
            framebufferAttachment.depth.image.destroy(*mContext);
            framebufferAttachment.depth.imageView.destroy(mContext->getDevice());
        }

        for (auto& framebuffer : mFrameBuffers) {
            framebuffer.destroy(mContext->getDevice());
        }

        mObjectManager.destroy();
        mVertexShader.destroy(mContext->getDevice());
        mFragmentShader.destroy(mContext->getDevice());
        vkFreeCommandBuffers(mContext->getDevice(), mContext->getGraphicsCommandPool().getHandler(), static_cast<uint32_t>(mCommandBuffers.size()), mCommandBuffers.data());
        mPipeline.destroy(mContext->getDevice());
        mRenderPass.destroy(mContext->getDevice());
        mSwapChain.destroy(mContext->getDevice());
        vkDestroyDescriptorPool(mContext->getDevice(), mDescriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(mContext->getDevice(), mCameraDescriptorSetLayout, nullptr);
        vkDestroySemaphore(mContext->getDevice(), mImageAvailableSemaphore, nullptr);
        vkDestroySemaphore(mContext->getDevice(), mRenderFinishedSemaphore, nullptr);
        for (size_t i{0};i < mFences.size();++i) {
            vkDestroyFence(mContext->getDevice(), mFences[i], nullptr);
        }
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

void Renderer::update(double dt) {
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

    if (mIsFenceSubmitted[mNextImageIndex]) {
        if (vkGetFenceStatus(mContext->getDevice(), mFences[mNextImageIndex]) == VK_NOT_READY) {
            std::cout << "Fence #" << mNextImageIndex << " not ready" << std::endl;
            vkWaitForFences(mContext->getDevice(), 1, &mFences[mNextImageIndex], VK_TRUE, 0);
            vkResetFences(mContext->getDevice(), 1, &mFences[mNextImageIndex]);
        } else {
            vkResetFences(mContext->getDevice(), 1, &mFences[mNextImageIndex]);
        }
    }

    updateUniformBuffer(mNextImageIndex);
    for (auto& o : mObjects) {
        o->getTransform().rotate(3.14159265 * dt, glm::vec3(0.0, 0.0, 1.0));
    }
    mObjectManager.updateUniformBuffer();
    if (mCommandBufferNeedUpdate[mNextImageIndex]) {
        updateCommandBuffer(mNextImageIndex);
        mCommandBufferNeedUpdate[mNextImageIndex] = false;
    }
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

void Renderer::createRenderPass() {
    mFramebufferAttachments.resize(mSwapChain.getImageCount());
    
    VkMemoryRequirements memoryRequirements;
    VkImageSubresourceRange subresourceRange{};
    for (auto& framebufferAttachment : mFramebufferAttachments) {
        framebufferAttachment.normal.image.setImageType(VK_IMAGE_TYPE_2D);
        framebufferAttachment.normal.image.setFormat(VK_FORMAT_R8G8B8A8_UNORM);
        framebufferAttachment.normal.image.setExtent({mExtent.width, mExtent.height, 1});
        framebufferAttachment.normal.image.setMipLevels(1);
        framebufferAttachment.normal.image.setArrayLayers(1);
        framebufferAttachment.normal.image.setSamples(VK_SAMPLE_COUNT_1_BIT);
        framebufferAttachment.normal.image.setTiling(VK_IMAGE_TILING_OPTIMAL);
        framebufferAttachment.normal.image.setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        framebufferAttachment.normal.image.setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED);
        framebufferAttachment.normal.image.create(*mContext);
        vkGetImageMemoryRequirements(mContext->getDevice(), framebufferAttachment.normal.image.getHandler(), &memoryRequirements);
        mContext->getMemoryManager().allocateForImage(
            framebufferAttachment.normal.image.getHandler(),
            memoryRequirements,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        framebufferAttachment.normal.imageView.setImageViewType(VK_IMAGE_VIEW_TYPE_2D);
        framebufferAttachment.normal.imageView.setFormat(VK_FORMAT_R8G8B8A8_UNORM);
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;
        framebufferAttachment.normal.imageView.setSubresourceRange(subresourceRange);
        framebufferAttachment.normal.imageView.setImage(framebufferAttachment.normal.image.getHandler());
        framebufferAttachment.normal.imageView.create(mContext->getDevice());

        framebufferAttachment.depth.image.setImageType(VK_IMAGE_TYPE_2D);
        framebufferAttachment.depth.image.setFormat(findDepthFormat());
        framebufferAttachment.depth.image.setExtent({mExtent.width, mExtent.height, 1});
        framebufferAttachment.depth.image.setMipLevels(1);
        framebufferAttachment.depth.image.setArrayLayers(1);
        framebufferAttachment.depth.image.setSamples(VK_SAMPLE_COUNT_1_BIT);
        framebufferAttachment.depth.image.setTiling(VK_IMAGE_TILING_OPTIMAL);
        framebufferAttachment.depth.image.setUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        framebufferAttachment.depth.image.setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED);
        framebufferAttachment.depth.image.create(*mContext);
        vkGetImageMemoryRequirements(mContext->getDevice(), framebufferAttachment.depth.image.getHandler(), &memoryRequirements);
        mContext->getMemoryManager().allocateForImage(
            framebufferAttachment.depth.image.getHandler(),
            memoryRequirements,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        framebufferAttachment.depth.imageView.setImageViewType(VK_IMAGE_VIEW_TYPE_2D);
        framebufferAttachment.depth.imageView.setFormat(findDepthFormat());
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;
        framebufferAttachment.depth.imageView.setSubresourceRange(subresourceRange);
        framebufferAttachment.depth.imageView.setImage(framebufferAttachment.depth.image.getHandler());
        framebufferAttachment.depth.imageView.create(mContext->getDevice());
    }

    ColorAttachment colorAttachment[2];
    colorAttachment[0].setFormat(mSwapChain.getFormat());
    colorAttachment[0].setSamples(VK_SAMPLE_COUNT_1_BIT);
    colorAttachment[0].setLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR);
    colorAttachment[0].setStoreOp(VK_ATTACHMENT_STORE_OP_STORE);
    colorAttachment[0].setStencilLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE);
    colorAttachment[0].setStencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE);
    colorAttachment[0].setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED);
    colorAttachment[0].setFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    colorAttachment[0].setReferenceIndex(0);

    colorAttachment[1].setFormat(VK_FORMAT_R8G8B8A8_UNORM);
    colorAttachment[1].setSamples(VK_SAMPLE_COUNT_1_BIT);
    colorAttachment[1].setLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR);
    colorAttachment[1].setStoreOp(VK_ATTACHMENT_STORE_OP_STORE);
    colorAttachment[1].setStencilLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE);
    colorAttachment[1].setStencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE);
    colorAttachment[1].setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED);
    colorAttachment[1].setFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    colorAttachment[1].setReferenceIndex(1);

    DepthAttachment depthAttachment;
    depthAttachment.setFormat(findDepthFormat());
    depthAttachment.setSamples(VK_SAMPLE_COUNT_1_BIT);
    depthAttachment.setLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR);
    depthAttachment.setStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE);
    depthAttachment.setStencilLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE);
    depthAttachment.setStencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE);
    depthAttachment.setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED);
    depthAttachment.setFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    depthAttachment.setReferenceIndex(2);

    std::vector<Attachment> attachments = {colorAttachment[0], colorAttachment[1], depthAttachment};

    mRenderPass.setAttachments(attachments);

    Subpass subpass;
    subpass.setBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    std::vector<VkAttachmentReference> colorAttachments = {
        colorAttachment[0].getReference(),
        colorAttachment[1].getReference()
    };

    subpass.setColorAttachments(colorAttachments);
    subpass.setDepthAttachment(depthAttachment.getReference());

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
        mObjectManager.getDescriptorSetLayout(),
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

void Renderer::createFramebuffers() {
    mFrameBuffers.resize(mSwapChain.getImageCount());

    for (size_t i{0}; i < mSwapChain.getImageCount();++i) {
        std::vector<VkImageView> attachments = {
            mSwapChain.getImageView(i),
            mFramebufferAttachments[i].normal.imageView.getHandler(),
            mFramebufferAttachments[i].depth.imageView.getHandler(),
        };

        mFrameBuffers[i].setRenderPass(mRenderPass.getHandler());
        mFrameBuffers[i].setAttachments(attachments);
        mFrameBuffers[i].setWidth(mExtent.width);
        mFrameBuffers[i].setHeight(mExtent.height);
        mFrameBuffers[i].setLayers(1);
        mFrameBuffers[i].create(mContext->getDevice());
    }
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
    mCommandBufferNeedUpdate.resize(mSwapChain.getImageCount(), true);

    Commands::allocateBuffers(mContext->getDevice(), mContext->getGraphicsCommandPool(), mCommandBuffers);
}

void Renderer::createCameraUniformBuffers() {
    mCameraUniformBuffers.resize(mSwapChain.getImageCount());
    VkDeviceSize bufferSize = sizeof(RenderInfo);

    for (size_t i{0};i < mCameraUniformBuffers.size();++i) {
        BufferHelper::createBuffer(*mContext,
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
        bufferInfo.range = sizeof(RenderInfo);

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
    Commands::begin(mCommandBuffers[index], VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = mRenderPass.getHandler();
    renderPassInfo.framebuffer = mFrameBuffers[index].getHandler();

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = mSwapChain.getExtent();

    renderPassInfo.clearValueCount = static_cast<uint32_t>(mClearValues.size());
    renderPassInfo.pClearValues = mClearValues.data();

    vkCmdBeginRenderPass(mCommandBuffers[index], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(mCommandBuffers[index], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline.getHandler());
    
    vkCmdBindDescriptorSets(mCommandBuffers[index],
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            mPipeline.getLayout().getHandler(),
                            1, 1, &mCameraDescriptorSets[index],
                            0, nullptr);

    mObjectManager.render(mCommandBuffers[index], mPipeline.getLayout().getHandler());    

    vkCmdEndRenderPass(mCommandBuffers[index]);

    if (vkEndCommandBuffer(mCommandBuffers[index]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffers");
    }
}

void Renderer::updateUniformBuffer(uint32_t index) {
    RenderInfo renderInfo;
    renderInfo.proj = mCamera->getProj();
    renderInfo.view = mCamera->getView();
    renderInfo.cameraPosition = glm::vec4(mCamera->getPosition(), 1.0);
    renderInfo.lightPosition = glm::vec4(mLight->position, 1.0);

    void* data;
    mContext->getMemoryManager().mapMemory(mCameraUniformBuffers[index], sizeof(RenderInfo), &data);
    memcpy(data, &renderInfo, sizeof(RenderInfo));
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