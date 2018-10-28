#include "renderer/Renderer.hpp"

#include "vulkan/ColorAttachment.hpp"
#include "vulkan/DepthAttachment.hpp"
#include "vulkan/Subpass.hpp"
#include "vulkan/SubpassDependency.hpp"
#include "vulkan/Image.hpp"
#include "vulkan/UniformBufferObject.hpp"

void Renderer::create(VkInstance instance) {
    mInstance = instance;

    createSurface();
    mSwapChain.query(mWindow, mPhysicalDevice, mDevice, mSurface);
    createRenderPass();
    createDepthResources();
    mSwapChain.create(mWindow, mPhysicalDevice, mDevice,
                      mSurface, mIndices, mDepthImageView,
                      mRenderPass);
    mExtent = mSwapChain.getExtent();
    mCommandPool.create(mDevice, mIndices);
    createGraphicsPipeline();
    createDescriptorPool();
    createDescriptorSetLayout();
    createDescriptorSets();
}

void Renderer::destroy() {

}

void Renderer::render() {

}

StaticObjectsManager& Renderer::getStaticObjectManager() {
    return mStaticObjectManager;
}

void Renderer::createSurface() {
    if (glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create the window surface");
    }
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

void Renderer::createFrameBuffers() {

}

void Renderer::createDepthResources() {
    VkFormat format = findDepthFormat();

    Image::create(
        mDevice, *mMemoryManager,
        mSwapChain.getExtent().width, mSwapChain.getExtent().height,
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

}

void Renderer::createSemaphores() {

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