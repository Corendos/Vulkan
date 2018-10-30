#include "renderer/Renderer.hpp"

#include "vulkan/ColorAttachment.hpp"
#include "vulkan/DepthAttachment.hpp"
#include "vulkan/Subpass.hpp"
#include "vulkan/SubpassDependency.hpp"
#include "vulkan/Image.hpp"
#include "vulkan/UniformBufferObject.hpp"
#include "vulkan/Commands.hpp"

Renderer::Renderer() {
    mVertexShader = Shader(shaderPath + "vert.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
    mFragmentShader = Shader(shaderPath + "fragColor.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");
}

void Renderer::create(VkInstance instance,
                      GLFWwindow* window,
                      VkPhysicalDevice physicalDevice,
                      VkDevice device,
                      VkSurfaceKHR surface,
                      QueueFamilyIndices indices,
                      VkQueue graphicsQueue,
                      VkQueue presentQueue,
                      MemoryManager& memoryManager) {
    if (mCreated) {
        return;
    }

    mInstance = instance;
    mWindow = window;
    mPhysicalDevice = physicalDevice;
    mDevice = device;
    mSurface = surface;
    mIndices = indices;
    mGraphicsQueue = graphicsQueue;
    mPresentQueue =  presentQueue;
    mMemoryManager = &memoryManager;

    mSwapChain.query(mWindow, mPhysicalDevice, mDevice, mSurface);
    mExtent = mSwapChain.getExtent();
    mCommandPool.create(mDevice, mIndices);
    createRenderPass();
    createDepthResources();
    mSwapChain.create(mWindow, mPhysicalDevice, mDevice,
                      mSurface, mIndices, mDepthImageView,
                      mRenderPass);
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createDescriptorPool();
    mStaticObjectManager.create(*this);
    createDescriptorSets();
    createCommandBuffers();
    createSemaphores();

    mCreated = true;
    mRecreated = true;
}

void Renderer::recreate() {
    int width{0}, height{0};
    while(width == 0 || height == 0) {
        glfwGetFramebufferSize(mWindow, &width, &height);
        glfwWaitEvents();
    }
    
    vkDeviceWaitIdle(mDevice);

    vkDestroyImageView(mDevice, mDepthImageView, nullptr);
    mMemoryManager->freeImage(mDepthImage);
    vkFreeCommandBuffers(mDevice, mCommandPool.getHandler(),
                         static_cast<uint32_t>(mCommandBuffers.size()),
                         mCommandBuffers.data());

    mPipeline.destroy(mDevice);
    mRenderPass.destroy(mDevice);
    mSwapChain.destroy(mDevice);

    mSwapChain.query(mWindow, mPhysicalDevice, mDevice, mSurface);
    mExtent = mSwapChain.getExtent();
    createRenderPass();
    createDepthResources();
    mSwapChain.create(mWindow, mPhysicalDevice, mDevice,
                      mSurface, mIndices, mDepthImageView,
                      mRenderPass);
    createRenderPass();
    createGraphicsPipeline();
    createCommandBuffers();
    mCamera->setExtent(mSwapChain.getExtent());
}

void Renderer::destroy() {
    if (mCreated) {
        mVertexShader.destroy(mDevice);
        mFragmentShader.destroy(mDevice);
        mMemoryManager->freeImage(mDepthImage);
        vkDestroyImageView(mDevice, mDepthImageView, nullptr);
        vkFreeCommandBuffers(mDevice, mCommandPool.getHandler(), static_cast<uint32_t>(mCommandBuffers.size()), mCommandBuffers.data());
        mPipeline.destroy(mDevice);
        mRenderPass.destroy(mDevice);
        mSwapChain.destroy(mDevice);
        mStaticObjectManager.destroy();
        vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);
        vkDestroySemaphore(mDevice, mImageAvailableSemaphore, nullptr);
        vkDestroySemaphore(mDevice, mRenderFinishedSemaphore, nullptr);
        mCommandPool.destroy(mDevice);
        vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
        mCreated = false;
    }
}

void Renderer::render() {
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(mDevice, mSwapChain.getHandler(), std::numeric_limits<uint64_t>::max(),
        mImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swap chain image");
    }

    mStaticObjectManager.update(*mCamera);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {mImageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mCommandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {mRenderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {mSwapChain.getHandler()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    result = vkQueuePresentKHR(mGraphicsQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreate();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("  Failed to present image");
    }
}

void Renderer::setCamera(Camera& camera) {
    mCamera = &camera;
}

MemoryManager& Renderer::getMemoryManager() {
    return *mMemoryManager;
}

VkDevice Renderer::getDevice() const {
    return mDevice;
}

VkQueue Renderer::getGraphicsQueue() const {
    return mGraphicsQueue;
}

CommandPool& Renderer::getCommandPool() {
    return mCommandPool;
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
    
    mRenderPass.create(mDevice);
}

void Renderer::createGraphicsPipeline() {
    mVertexShader.create(mDevice);
    mFragmentShader.create(mDevice);

    mPipeline.getLayout().addDescriptorSetLayout(mDescriptorSetLayout);
    mPipeline.addShader(mVertexShader);
    mPipeline.addShader(mFragmentShader);
    mPipeline.setRenderPass(mRenderPass);
    mPipeline.setExtent(mExtent);
    mPipeline.create(mDevice);
}

void Renderer::createDepthResources() {
    VkFormat format = findDepthFormat();

    Image::create(
        mDevice, *mMemoryManager,
        mExtent.width, mExtent.height,
        format, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        mDepthImage
    );

    mDepthImageView = Image::createImageView(mDevice, mDepthImage,
                                             format,
                                             VK_IMAGE_ASPECT_DEPTH_BIT);
    
    Image::transitionImageLayout(mDevice, mCommandPool,
                                 mGraphicsQueue, mDepthImage,
                                 format, VK_IMAGE_LAYOUT_UNDEFINED,
                                 VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void Renderer::createDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = /*static_cast<uint32_t>(mSwapChain.getImageCount())*/100;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = /*static_cast<uint32_t>(mSwapChain.getImageCount())*/100;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = /*static_cast<uint32_t>(mSwapChain.getImageCount())*/10;

    if (vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool");
    }
}

void Renderer::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo colorLayoutInfo{};
    colorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    colorLayoutInfo.bindingCount = 1;
    colorLayoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(mDevice, &colorLayoutInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout");
    }
}

void Renderer::createDescriptorSets() {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = mDescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &mDescriptorSetLayout;

    if (vkAllocateDescriptorSets(mDevice, &allocInfo, &mDescriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor sets");
    }

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = mStaticObjectManager.getUniformBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = mDescriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;
    descriptorWrite.pImageInfo = nullptr;
    descriptorWrite.pTexelBufferView = nullptr;

    VkWriteDescriptorSet sets[] = {descriptorWrite};

    vkUpdateDescriptorSets(mDevice, 1, sets, 0, nullptr);
}

void Renderer::createCommandBuffers() {
    mCommandBuffers.resize(mSwapChain.getImageCount());

    Commands::allocateBuffers(mDevice, mCommandPool, mCommandBuffers);

    for(size_t i{0};i < mCommandBuffers.size();++i) {
        Commands::begin(mCommandBuffers[i], VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = mRenderPass.getHandler();
        renderPassInfo.framebuffer = mSwapChain.getFramebuffers()[i];

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = mSwapChain.getExtent();

        std::array<VkClearValue, 2> clearValues;
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(mCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline.getHandler());
        
        VkBuffer vertexBuffers[] = {mStaticObjectManager.getVertexBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(mCommandBuffers[i], 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(mCommandBuffers[i], mStaticObjectManager.getIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);
        
        VkDescriptorSet desc[] = {mDescriptorSet};
        vkCmdBindDescriptorSets(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline.getLayout().getHandler(),
            0, 1, desc, 0, nullptr);
        
        vkCmdDrawIndexed(mCommandBuffers[i], mStaticObjectManager.getIndiceCount(), 1, 0, 0, 0);        

        vkCmdEndRenderPass(mCommandBuffers[i]);

        if (vkEndCommandBuffer(mCommandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffers");
        }
    }
}

void Renderer::createSemaphores() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    if ((vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mImageAvailableSemaphore) != VK_SUCCESS) ||
        (vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mRenderFinishedSemaphore) != VK_SUCCESS)) {
        throw std::runtime_error("Failed to create semaphores");
    }
}


VkFormat Renderer::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, format, &properties);

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