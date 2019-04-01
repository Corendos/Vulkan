#include "renderer/Renderer.hpp"

#include <chrono>
#include <iostream>

#include "vulkan/Subpass.hpp"
#include "vulkan/SubpassDependency.hpp"
#include "vulkan/UniformBufferObject.hpp"
#include "vulkan/Commands.hpp"
#include "vulkan/buffer/BufferHelper.hpp"
#include "vulkan/image/ImageHelper.hpp"
#include "colors/Color.hpp"
#include "renderer/RenderInfo.hpp"
#include "environment.hpp"

Renderer::Renderer() {
    mVertexShader = Shader(shaderPath + "vert.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
    mFragmentShader = Shader(shaderPath + "frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");

    mClearValues[0].color = {0.325f, 0.694f, 0.937f, 1.0f};
    mClearValues[1].color = {0.0f, 0.0f, 0.0f, 0.0f};
    mClearValues[2].depthStencil = {1.0f, 0};

    mNextImageIndex = 0;
}

void Renderer::create(VulkanContext& context, TextureManager& textureManager, MeshManager& meshManager) {
    if (mCreated) {
        return;
    }

    mContext = &context;
    mTextureManager = &textureManager;
    mMeshManager = &meshManager;

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
    createCommandPools();
    createCommandBuffers();
    createSemaphores();
    createFences();

    mCreated = true;
}

void Renderer::recreate() {
    int width{0}, height{0};
    while(width == 0 || height == 0) {
        glfwGetFramebufferSize(mContext->getWindow(), &width, &height);
        glfwWaitEvents();
    }
    
    vkDeviceWaitIdle(mContext->getDevice());

    /* Destroying resources that need to be recreated */

    for (auto& framebufferAttachment : mFramebufferAttachments) {
        framebufferAttachment.normal.image.destroy(*mContext);
        framebufferAttachment.normal.imageView.destroy(mContext->getDevice());
        framebufferAttachment.depth.image.destroy(*mContext);
        framebufferAttachment.depth.imageView.destroy(mContext->getDevice());
    }

    for (auto& framebuffer : mFrameBuffers) {
        framebuffer.destroy(mContext->getDevice());
    }
    
    mSwapChain.destroy(mContext->getDevice());
    mRenderPass.destroy(mContext->getDevice());
    mPipeline.destroy(mContext->getDevice());

    /* Recreate the resources */

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
        mVertexShader.destroy(mContext->getDevice());
        mFragmentShader.destroy(mContext->getDevice());

        for (auto& commandPool : mCommandPools) {
            vkDestroyCommandPool(mContext->getDevice(), commandPool, nullptr);
        }

        vkDestroyDescriptorPool(mContext->getDevice(), mDescriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(mContext->getDevice(), mCameraDescriptorSetLayout, nullptr);

        mSwapChain.destroy(mContext->getDevice());
        mRenderPass.destroy(mContext->getDevice());
        mPipeline.destroy(mContext->getDevice());
        
        vkDestroySemaphore(mContext->getDevice(), mImageAvailableSemaphore, nullptr);
        for (size_t i{0};i < mRenderFinishedSemaphores.size();++i) {
            vkDestroySemaphore(mContext->getDevice(), mRenderFinishedSemaphores[i], nullptr);
        }

        for (size_t i{0};i < mFencesInfo.size();++i) {
            vkDestroyFence(mContext->getDevice(), mFencesInfo[i].fence, nullptr);
        }

        mCreated = false;
    }
}

void Renderer::render() {
    if (mBypassRendering) {
        return;
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    mToWaitSemaphores.push_back(mImageAvailableSemaphore);
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = mToWaitSemaphores.size();
    submitInfo.pWaitSemaphores = mToWaitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mCommandBuffers[mNextImageIndex];

    VkSemaphore signalSemaphores[] = {mRenderFinishedSemaphores[mNextImageIndex]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(mContext->getGraphicsQueue(), 1, &submitInfo, mFencesInfo[mNextImageIndex].fence) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer");
    }

    mFencesInfo[mNextImageIndex].submitted = true;

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
    acquireNextImage();

    mToWaitSemaphores.clear();

    mMeshManager->update(mNextImageIndex);

    VkCommandBuffer staticBuffer = mMeshManager->render(
        mRenderPass.getHandler(), mFrameBuffers[mNextImageIndex].getHandler(),
        mCommandPools[mNextImageIndex], mCameraDescriptorSets[mNextImageIndex],
        mPipeline.getLayout().getHandler(), mPipeline.getHandler(), mNextImageIndex);

    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = mCommandPools[mNextImageIndex];
    allocateInfo.commandBufferCount = 1;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    vkAllocateCommandBuffers(mContext->getDevice(), &allocateInfo, &mCommandBuffers[mNextImageIndex]);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(mCommandBuffers[mNextImageIndex], &beginInfo);

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = mRenderPass.getHandler();
    renderPassInfo.framebuffer = mFrameBuffers[mNextImageIndex].getHandler();

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = mSwapChain.getExtent();

    renderPassInfo.clearValueCount = static_cast<uint32_t>(mClearValues.size());
    renderPassInfo.pClearValues = mClearValues.data();

    vkCmdBeginRenderPass(mCommandBuffers[mNextImageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    vkCmdExecuteCommands(mCommandBuffers[mNextImageIndex], 1, &staticBuffer);

    vkCmdEndRenderPass(mCommandBuffers[mNextImageIndex]);

    vkEndCommandBuffer(mCommandBuffers[mNextImageIndex]);

    updateUniformBuffer(mNextImageIndex);
    waitForFence();
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

SwapChain& Renderer::getSwapChain() {
    return mSwapChain;
}

void Renderer::acquireNextImage() {
    mBypassRendering = false;
    VkResult result = vkAcquireNextImageKHR(
        mContext->getDevice(), mSwapChain.getHandler(), std::numeric_limits<uint64_t>::max() - 1,
        mImageAvailableSemaphore, VK_NULL_HANDLE, &mNextImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate();
        mBypassRendering = true;
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swap chain image");
    }
}

void Renderer::waitForFence() {
    if (mFencesInfo[mNextImageIndex].submitted) {
        if (vkGetFenceStatus(mContext->getDevice(), mFencesInfo[mNextImageIndex].fence) == VK_NOT_READY) {
            vkWaitForFences(mContext->getDevice(), 1, &mFencesInfo[mNextImageIndex].fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
        }
        vkResetFences(mContext->getDevice(), 1, &mFencesInfo[mNextImageIndex].fence);
    }
}

void Renderer::createRenderPass() {
    /* Create framebuffers */
    mFramebufferAttachments.resize(mSwapChain.getImageCount());
    
    VkMemoryRequirements memoryRequirements;
    VkImageSubresourceRange subresourceRange{};
    for (auto& framebufferAttachment : mFramebufferAttachments) {
        /* Create the normal attachment */
        framebufferAttachment.normal = createAttachment(
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT);
        
        /* Create the depth attachment */
        framebufferAttachment.depth = createAttachment(
            findDepthFormat(),
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    /* Create the render pass attachments */
    std::vector<Attachment> attachments(3);
    std::vector<VkAttachmentReference> attachmentReferences(3);
    attachments[0].setFormat(mSwapChain.getFormat());
    attachments[0].setSamples(VK_SAMPLE_COUNT_1_BIT);
    attachments[0].setLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR);
    attachments[0].setStoreOp(VK_ATTACHMENT_STORE_OP_STORE);
    attachments[0].setStencilLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE);
    attachments[0].setStencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE);
    attachments[0].setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED);
    attachments[0].setFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    attachmentReferences[0].attachment = 0;
    attachmentReferences[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    attachments[1].setFormat(VK_FORMAT_R8G8B8A8_UNORM);
    attachments[1].setSamples(VK_SAMPLE_COUNT_1_BIT);
    attachments[1].setLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR);
    attachments[1].setStoreOp(VK_ATTACHMENT_STORE_OP_STORE);
    attachments[1].setStencilLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE);
    attachments[1].setStencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE);
    attachments[1].setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED);
    attachments[1].setFinalLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    attachmentReferences[1].attachment = 1;
    attachmentReferences[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    attachments[2].setFormat(findDepthFormat());
    attachments[2].setSamples(VK_SAMPLE_COUNT_1_BIT);
    attachments[2].setLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR);
    attachments[2].setStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE);
    attachments[2].setStencilLoadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE);
    attachments[2].setStencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE);
    attachments[2].setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED);
    attachments[2].setFinalLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    attachmentReferences[2].attachment = 2;
    attachmentReferences[2].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    mRenderPass.setAttachments(attachments);

    Subpass subpass;
    subpass.setBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);

    std::vector<VkAttachmentReference> colorAttachments = {
        attachmentReferences[0],
        attachmentReferences[1]
    };
    subpass.setColorAttachments(colorAttachments);
    subpass.setDepthAttachment(attachmentReferences[2]);

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
        mMeshManager->getDescriptorSetLayout(),
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
}

void Renderer::createCameraUniformBuffers() {
    mCameraUniformBuffers.resize(mSwapChain.getImageCount());
    VkDeviceSize bufferSize = sizeof(RenderInfo);

    for (size_t i{0};i < mCameraUniformBuffers.size();++i) {
        BufferHelper::createBuffer(*mContext,
                               bufferSize,
                               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               VK_SHARING_MODE_EXCLUSIVE,
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
                               mCameraUniformBuffers[i],
                               "Camera Uniform Buffer");
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

    mRenderFinishedSemaphores.resize(mSwapChain.getImageCount());

    for (size_t i{0};i < mSwapChain.getImageCount();++i) {
        if (vkCreateSemaphore(mContext->getDevice(), &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create semaphores");
        }
    }

    if (vkCreateSemaphore(mContext->getDevice(), &semaphoreInfo, nullptr, &mImageAvailableSemaphore) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create semaphores");
    }
}

void Renderer::createFences() {
    mFencesInfo.resize(mSwapChain.getImageCount());
    VkFenceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    for (size_t i{0};i < mFencesInfo.size();++i) {
        if (vkCreateFence(mContext->getDevice(), &createInfo, nullptr, &mFencesInfo[i].fence) != VK_SUCCESS) {
            throw std::runtime_error("failed to create fences");
        }
    }
}

void Renderer::createCommandPools() {
    mCommandPools.resize(mSwapChain.getImageCount());
    VkCommandPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.queueFamilyIndex = mContext->getQueueFamilyIndices().graphicsFamily.value();
    for (size_t i{0};i < mSwapChain.getImageCount();++i) {
        VkResult result = vkCreateCommandPool(mContext->getDevice(), &createInfo, nullptr, &mCommandPools.at(i));
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pools");
        }
    }
}

FrameBufferAttachment Renderer::createAttachment(VkFormat format,
                                                 VkImageUsageFlags usage,
                                                 VkImageAspectFlags aspect) {
    VkMemoryRequirements memoryRequirements;
    VkImageSubresourceRange subresourceRange{};

    FrameBufferAttachment attachment;
    attachment.image.setImageType(VK_IMAGE_TYPE_2D);
    attachment.image.setFormat(format);
    attachment.image.setExtent({mExtent.width, mExtent.height, 1});
    attachment.image.setMipLevels(1);
    attachment.image.setArrayLayers(1);
    attachment.image.setSamples(VK_SAMPLE_COUNT_1_BIT);
    attachment.image.setTiling(VK_IMAGE_TILING_OPTIMAL);
    attachment.image.setUsage(usage);
    attachment.image.setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED);
    attachment.image.create(*mContext);
    vkGetImageMemoryRequirements(mContext->getDevice(), attachment.image.getHandler(), &memoryRequirements);
    mContext->getMemoryManager().allocateForImage(
        attachment.image.getHandler(),
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "attachment"
    );

    attachment.imageView.setImageViewType(VK_IMAGE_VIEW_TYPE_2D);
    attachment.imageView.setFormat(format);
    subresourceRange.aspectMask = aspect;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 1;
    attachment.imageView.setSubresourceRange(subresourceRange);
    attachment.imageView.setImage(attachment.image.getHandler());
    attachment.imageView.create(mContext->getDevice());

    return attachment;
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