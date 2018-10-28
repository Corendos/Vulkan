#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#include "BasicLogger.hpp"
#include "PrintHelper.hpp"
#include "vulkan/Vulkan.hpp"
#include "vulkan/BasicPhysicalDevicePicker.hpp"
#include "vulkan/Vertex.hpp"
#include "vulkan/UniformBufferObject.hpp"
#include "vulkan/Image.hpp"
#include "vulkan/ColorAttachment.hpp"
#include "vulkan/DepthAttachment.hpp"
#include "vulkan/SubpassDependency.hpp"

Vulkan::Vulkan() : mMemoryManager(mPhysicalDevice, mDevice) {}

void Vulkan::init(GLFWwindow* window, int width, int height) {
    mWindowSize = {width, height};
    mWindow = window;

    createInstance();
    createSurface();
    setupDebugCallback();
    pickPhysicalDevice();
    createLogicalDevice();
    mMemoryManager.init();
    createObjects();
    mRenderer.create(mInstance, mWindow, mPhysicalDevice,
                     mDevice, mSurface, mIndices,
                     mGraphicsQueue, mPresentQueue, mMemoryManager);
    mRenderer.setCamera(*mCamera);
}

void Vulkan::cleanup() {
    vkDeviceWaitIdle(mDevice);
    mRenderer.destroy();

    for (size_t i{0};i < mUniformBuffers.size();++i) {
        mMemoryManager.freeBuffer(mUniformBuffers[i]);
    }

    mMemoryManager.freeBuffer(mVertexBuffer);
    mMemoryManager.freeBuffer(mIndicesBuffer);
    mMemoryManager.freeImage(mTextureImage);

    vkDestroySampler(mDevice, mTextureSampler, nullptr);
    vkDestroyImageView(mDevice, mTextureImageView, nullptr);

    if (enableValidationLayers) {
        destroyDebugUtilsMessengerEXT(mInstance, mCallback, nullptr);
    }

    mMemoryManager.cleanup();
    mMemoryManager.memoryCheckLog();
    vkDestroyDevice(mDevice, nullptr);
    vkDestroyInstance(mInstance, nullptr);
}

VkDevice Vulkan::getDevice() const {
    return mDevice;
}

CommandPool& Vulkan::getCommandPool() {
    return mCommandPool;
}

VkQueue Vulkan::getGraphicsQueue() const {
    return mGraphicsQueue;
}

MemoryManager& Vulkan::getMemoryManager() {
    return mMemoryManager;
}

void Vulkan::drawFrame() {
    mRenderer.render();
}

void Vulkan::createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("Validation layers requested, but not available");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Api";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if(vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan instance");
    }
}

void Vulkan::setupDebugCallback() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = 
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;

    if (createDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mCallback) != VK_SUCCESS) {
        throw std::runtime_error("Failed to set up a debug callback");
    }
}

void Vulkan::createSurface() {
    if (glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create the window surface");
    }
}

void Vulkan::pickPhysicalDevice() {
    BasicPhysicalDevicePicker devicePicker{mInstance, mSurface, deviceExtension};

    auto pickedDevice = devicePicker.pick();

    if (!pickedDevice.isComplete()) {
        throw std::runtime_error("Failed to find GPUs with required features");
    }

    mPhysicalDevice = pickedDevice.physicalDevice;
    mIndices = pickedDevice.queueFamilyIndices;
}

void Vulkan::createLogicalDevice() {
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        mIndices.graphicsFamily.value(),
        mIndices.presentFamily.value(),
    };

    float queuePriority{1.0f};
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtension.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtension.data();

    if (enableValidationLayers) {
        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        deviceCreateInfo.enabledLayerCount = 0;
    }

    if(vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, nullptr, &mDevice) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device");
    }

    vkGetDeviceQueue(mDevice, mIndices.graphicsFamily.value(), 0, &mGraphicsQueue);
    vkGetDeviceQueue(mDevice, mIndices.presentFamily.value(), 0, &mPresentQueue);
}

void Vulkan::createDepthResources() {
}

void Vulkan::createRenderPass() {
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

void Vulkan::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout");
    }

    VkDescriptorSetLayoutCreateInfo colorLayoutInfo{};
    colorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    colorLayoutInfo.bindingCount = 1;
    colorLayoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(mDevice, &colorLayoutInfo, nullptr, &mColorDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout");
    }
}

void Vulkan::createObjects() {
    mRenderer.getStaticObjectManager().addStaticObject(cube2);
    mRenderer.getStaticObjectManager().addStaticObject(cube);
}

void Vulkan::createGraphicsPipeline() {

    mGraphicsPipeline.getLayout().addDescriptorSetLayout(mDescriptorSetLayout);

    mGraphicsPipeline.setRenderPass(mRenderPass);
    mGraphicsPipeline.setExtent(mSwapChain.getExtent());
    mGraphicsPipeline.create(mDevice);

    mGraphicsPipeline2.getLayout().addDescriptorSetLayout(mColorDescriptorSetLayout);

    mGraphicsPipeline2.setRenderPass(mRenderPass);
    mGraphicsPipeline2.setExtent(mSwapChain.getExtent());
    mGraphicsPipeline2.create(mDevice);
}

void Vulkan::createFrameBuffers() {
}

void Vulkan::createTextureImage() {
    int textureWidth, textureHeight, textureChannels;
    std::string filePath = std::string(ROOT_PATH) + std::string("textures/texture.jpg");
    stbi_uc* pixels = stbi_load(filePath.c_str(), &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = textureWidth * textureHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image");
    }

    VkBuffer stagingBuffer;
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        stagingBuffer);
    
    void* data;
    mMemoryManager.mapMemory(stagingBuffer, imageSize, &data);
    memcpy(data, pixels, static_cast<uint32_t>(imageSize));
    mMemoryManager.unmapMemory(stagingBuffer);

    stbi_image_free(pixels);

    Image::create(
        mDevice, mMemoryManager,
        static_cast<uint32_t>(textureWidth), static_cast<uint32_t>(textureHeight),
        VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mTextureImage
    );

    transitionImageLayout(
        mTextureImage, VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    
    copyBufferToImage(
        stagingBuffer, mTextureImage,
        static_cast<uint32_t>(textureWidth),
        static_cast<uint32_t>(textureHeight)
    );

    transitionImageLayout(
        mTextureImage, VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    mMemoryManager.freeBuffer(stagingBuffer);
}

void Vulkan::createTextureImageView() {
    mTextureImageView = createImageView(mTextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Vulkan::createTextureSampler() {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(mDevice, &samplerInfo, nullptr, &mTextureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create sampler");
    }
}

void Vulkan::createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer);

    void* data;
    mMemoryManager.mapMemory(stagingBuffer, bufferSize, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    mMemoryManager.unmapMemory(stagingBuffer);

    createBuffer(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        mVertexBuffer);
    
    copyBuffer(stagingBuffer, mVertexBuffer, bufferSize);

    mMemoryManager.freeBuffer(stagingBuffer);
}

void Vulkan::createIndicesBuffer() {
    VkDeviceSize size = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;

    createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer);
    
    void* data;
    mMemoryManager.mapMemory(stagingBuffer, size, &data);
    memcpy(data, indices.data(), (size_t)size);
    mMemoryManager.unmapMemory(stagingBuffer);

    createBuffer(size,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        mIndicesBuffer);

    copyBuffer(stagingBuffer, mIndicesBuffer, size);

    mMemoryManager.freeBuffer(stagingBuffer);
}

void Vulkan::createUniformBuffer() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    mUniformBuffers.resize(mSwapChain.getImageCount());

    for (size_t i{0};i < mSwapChain.getImageCount();++i) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            mUniformBuffers[i]);
    }
}

void Vulkan::createDescriptorPool() {
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

void Vulkan::createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts{mSwapChain.getImageCount(), mDescriptorSetLayout};
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = mDescriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(mSwapChain.getImageCount());
    allocInfo.pSetLayouts = layouts.data();

    mDescriptorSets.resize(mSwapChain.getImageCount());
    if (vkAllocateDescriptorSets(mDevice, &allocInfo, mDescriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor sets");
    }

    for (size_t i{0};i < mSwapChain.getImageCount();++i) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = mUniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = mTextureImageView;
        imageInfo.sampler = mTextureSampler;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = mDescriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;
        descriptorWrites[0].pImageInfo = nullptr;
        descriptorWrites[0].pTexelBufferView = nullptr;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = mDescriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = nullptr;
        descriptorWrites[1].pImageInfo = &imageInfo;
        descriptorWrites[1].pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(mDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void Vulkan::createSODescriptorSets() {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = mDescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &mColorDescriptorSetLayout;

    if (vkAllocateDescriptorSets(mDevice, &allocInfo, &mSODescriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor sets");
    }

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = sObjectManager.getUniformBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = mSODescriptorSet;
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

void Vulkan::createCommandBuffers() {
    mCommandBuffers.resize(mSwapChainFrameBuffers.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = mCommandPool.getHandler();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) mCommandBuffers.size();

    if (vkAllocateCommandBuffers(mDevice, &allocInfo, mCommandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command buffers");
    }

    for(size_t i{0};i < mCommandBuffers.size();++i) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(mCommandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command info");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = mRenderPass.getHandler();
        renderPassInfo.framebuffer = mSwapChainFrameBuffers[i];

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = mSwapChain.getExtent();

        std::array<VkClearValue, 2> clearValues;
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(mCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline.getHandler());

        VkBuffer vertexBuffers[] = {mVertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(mCommandBuffers[i], 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(mCommandBuffers[i], mIndicesBuffer, 0, VK_INDEX_TYPE_UINT16);

        vkCmdBindDescriptorSets(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline.getLayout().getHandler(),
            0, 1, &mDescriptorSets[i], 0, nullptr);

        vkCmdDrawIndexed(mCommandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        
        vkCmdBindPipeline(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline2.getHandler());
        vertexBuffers[0] = sObjectManager.getVertexBuffer();
        vkCmdBindVertexBuffers(mCommandBuffers[i], 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(mCommandBuffers[i], sObjectManager.getIndexBuffer(), 0, VK_INDEX_TYPE_UINT16);
        VkDescriptorSet desc[] = {mSODescriptorSet};
        vkCmdBindDescriptorSets(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline2.getLayout().getHandler(),
            0, 1, desc, 0, nullptr);
        
        vkCmdDrawIndexed(mCommandBuffers[i], sObjectManager.getIndiceCount(), 1, 0, 0, 0);        

        vkCmdEndRenderPass(mCommandBuffers[i]);

        if (vkEndCommandBuffer(mCommandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffers");
        }
    }
}

void Vulkan::createSemaphores() {

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    if (
        (vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mImageAvailableSemaphore) != VK_SUCCESS) ||
        (vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mRenderFinishedSemaphore) != VK_SUCCESS)) {
            throw std::runtime_error("Failed to create semaphores");
        }
}

void Vulkan::recreateSwapChain() {
    int width{0}, height{0};
    while(width == 0 || height == 0) {
        glfwGetFramebufferSize(mWindow, &width, &height);
        glfwWaitEvents();
    }
    
    vkDeviceWaitIdle(mDevice);

    cleanupSwapChain();

    //mSwapChain.create(mPhysicalDevice, mDevice, mWindow, mSurface, mIndices);
    createDepthResources();
    createRenderPass();
    createGraphicsPipeline();
    createFrameBuffers();
    createCommandBuffers();
    mCamera->setExtent(mSwapChain.getExtent());
}

void Vulkan::cleanupSwapChain() {
    for (auto framebuffer : mSwapChainFrameBuffers) {
        vkDestroyFramebuffer(mDevice, framebuffer, nullptr);
    }

    vkFreeCommandBuffers(mDevice, mCommandPool.getHandler(),
        static_cast<uint32_t>(mCommandBuffers.size()), mCommandBuffers.data());

    mGraphicsPipeline.destroy(mDevice);
    mGraphicsPipeline2.destroy(mDevice);
    mRenderPass.destroy(mDevice);
    mSwapChain.destroy(mDevice);
}

bool Vulkan::checkValidationLayerSupport() {
    
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (VkLayerProperties layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }
    
    return true;
}

void Vulkan::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags properties, VkBuffer& buffer) {
    
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(mDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create vertex buffer");
    }

    VkMemoryRequirements memoryRequirements{};
    vkGetBufferMemoryRequirements(mDevice, buffer, &memoryRequirements);

    mMemoryManager.allocateForBuffer(buffer, memoryRequirements, properties);
}

void Vulkan::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void Vulkan::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(
        commandBuffer, buffer, image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &region);

    endSingleTimeCommands(commandBuffer);
}

VkCommandBuffer Vulkan::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandPool = mCommandPool.getHandler();
    allocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer{};
    vkAllocateCommandBuffers(mDevice, &allocateInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Vulkan::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(mGraphicsQueue);

    vkFreeCommandBuffers(mDevice, mCommandPool.getHandler(), 1, &commandBuffer);
}

void Vulkan::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier memoryBarrier{};
    memoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    memoryBarrier.oldLayout = oldLayout;
    memoryBarrier.newLayout = newLayout;
    memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    memoryBarrier.image = image;
    memoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    memoryBarrier.subresourceRange.baseMipLevel = 0;
    memoryBarrier.subresourceRange.levelCount = 1;
    memoryBarrier.subresourceRange.baseArrayLayer = 0;
    memoryBarrier.subresourceRange.layerCount = 1;
    memoryBarrier.srcAccessMask = 0;
    memoryBarrier.dstAccessMask = 0;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        memoryBarrier.srcAccessMask = 0;
        memoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        memoryBarrier.srcAccessMask = 0;
        memoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else {
        throw std::runtime_error("Unsupported layout transition");
    }

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        memoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (hasStencilComponent(format)) {
            memoryBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else {
        memoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &memoryBarrier
    );

    endSingleTimeCommands(commandBuffer);
}

VkImageView Vulkan::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags){
    VkImageView imageView;

    VkImageViewCreateInfo imageViewInfo{};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = image;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format = format;
    imageViewInfo.subresourceRange.aspectMask = aspectFlags;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.levelCount = 1;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(mDevice, &imageViewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image view");
    }

    return imageView;
}

std::vector<const char*> Vulkan::getRequiredExtensions() {
    uint32_t glfwExtensionCount{0};
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

uint32_t Vulkan::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memoryProperties);

    for (uint32_t i{0};i < memoryProperties.memoryTypeCount;++i) {
        if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find a suitable memory type");
}

VkFormat Vulkan::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
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

VkFormat Vulkan::findDepthFormat() {
    return findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

bool Vulkan::hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void Vulkan::requestResize() {
    mRenderer.recreate();
}

VKAPI_ATTR VkBool32 VKAPI_CALL Vulkan::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
        std::cout << "Validation layer(";
        switch(messageSeverity) {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                std::cout << "ERROR): ";
            break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                std::cout << "INFO): ";
            break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                std::cout << "VERBOSE): ";
            break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                std::cout << "WARNING): ";
            break;
            default:
                std::cout << "UNKNOWN): ";
            break;
        }
    std::cout << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

void Vulkan::updateUniformData(uint32_t imageIndex) {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    UniformBufferObject ubo{};
    ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(2.0 * cos(time * glm::radians(90.0f)), sin(time * glm::radians(90.0f)), 0.0f));
    ubo.view = mCamera->getView();
    ubo.proj = mCamera->getProj();

    void* data;
    mMemoryManager.mapMemory(mUniformBuffers[imageIndex], sizeof(ubo), &data);
    memcpy(data, &ubo, sizeof(ubo));
    mMemoryManager.unmapMemory(mUniformBuffers[imageIndex]);
}

void Vulkan::setCamera(Camera& camera) {
    mCamera = &camera;
}