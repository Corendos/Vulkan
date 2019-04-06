#include <set>
#include <cstring>
#include <iostream>

#include "vulkan/VulkanContext.hpp"
#include "utils.hpp"
#include "vulkan/BasicPhysicalDevicePicker.hpp"

VulkanContext::VulkanContext() : mMemoryManager(mPhysicalDevice, mDevice) {}

void VulkanContext::create(GLFWwindow* window) {
    mWindow = window;
    createInstance();
    createSurface();
    setupDebugCallback();
    pickPhysicalDevice();
    createLogicalDevice();
    createDescriptorPool();
    mMemoryManager.init();
}

void VulkanContext::destroy() {
    vkDeviceWaitIdle(mDevice);

    if (enableValidationLayers) {
        destroyDebugUtilsMessengerEXT(mInstance, mCallback, nullptr);
    }

    mMemoryManager.cleanup();
    mMemoryManager.memoryCheckLog();
    for (auto& pair : mTransferCommandPoolMap) {
        pair.second.destroy(mDevice);
    }
    mDescriptorPool.destroy(mDevice);
    vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
    vkDestroyDevice(mDevice, nullptr);
    mInstance.destroy();
}

vk::Instance VulkanContext::getInstance() const {
    return mInstance;
}

vk::PhysicalDevice VulkanContext::getPhysicalDevice() const {
    return mPhysicalDevice;
}

vk::Device VulkanContext::getDevice() const {
    return mDevice;
}

VkPhysicalDeviceLimits VulkanContext::getLimits() const {
    return mPhysicalDeviceLimits;
}

VkQueue VulkanContext::getPresentQueue() const {
    return mPresentQueue;
}

VkQueue VulkanContext::getTransferQueue() const {
    return mTransferQueue;
}

VkQueue VulkanContext::getGraphicsQueue() const {
    return mGraphicsQueue;
}

QueueFamilyIndices VulkanContext::getQueueFamilyIndices() const {
    return mIndices;
}

CommandPool& VulkanContext::getTransferCommandPool() {
    if (mTransferCommandPoolMap.find(std::this_thread::get_id()) == mTransferCommandPoolMap.end()) {
        CommandPool transferCommandPool;
        transferCommandPool.create(mDevice, mIndices.transferFamily.value());
        mTransferCommandPoolMap[std::this_thread::get_id()] = std::move(transferCommandPool);
    }
    return mTransferCommandPoolMap[std::this_thread::get_id()];
}

VkSurfaceKHR VulkanContext::getSurface() const {
    return mSurface;
}

GLFWwindow* VulkanContext::getWindow() const {
    return mWindow;
}

MemoryManager& VulkanContext::getMemoryManager() {
    return mMemoryManager;
}

DescriptorPool& VulkanContext::getDescriptorPool() {
    return mDescriptorPool;
}

void VulkanContext::createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("Validation layers requested, but not available");
    }

    vk::ApplicationInfo appInfo;
    appInfo.setPApplicationName("Vulkan Api");
    appInfo.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0));
    appInfo.setPEngineName("No Engine");
    appInfo.setEngineVersion(VK_MAKE_VERSION(1, 0, 0));
    appInfo.setApiVersion(VK_API_VERSION_1_0);

    vk::InstanceCreateInfo createInfo;
    createInfo.setPApplicationInfo(&appInfo);

    if (enableValidationLayers) {
        createInfo.setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()));
        createInfo.setPpEnabledLayerNames(validationLayers.data());
    }

    auto extensions = getRequiredExtensions();
    createInfo.setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()));
    createInfo.setPpEnabledExtensionNames(extensions.data());

    mInstance = vk::createInstance(createInfo);
}

void VulkanContext::setupDebugCallback() {
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

void VulkanContext::createSurface() {
    if (glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create the window surface");
    }
}

void VulkanContext::pickPhysicalDevice() {
    BasicPhysicalDevicePicker devicePicker{mInstance, mSurface, deviceExtension};

    auto pickedDevice = devicePicker.pick();

    if (!pickedDevice.isComplete()) {
        throw std::runtime_error("Failed to find GPUs with required features");
    }

    mPhysicalDevice = pickedDevice.physicalDevice;
    mIndices = pickedDevice.queueFamilyIndices;
    mPhysicalDeviceLimits = pickedDevice.limits;
}

void VulkanContext::createLogicalDevice() {
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        mIndices.graphicsFamily.value(),
        mIndices.presentFamily.value(),
    };

    float queuePriority{1.0f};
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.setQueueFamilyIndex(queueFamily);
        queueCreateInfo.setQueueCount(1);
        queueCreateInfo.setPQueuePriorities(&queuePriority);
        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::PhysicalDeviceFeatures deviceFeatures;
    deviceFeatures.setSamplerAnisotropy(VK_TRUE);

    vk::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.setPQueueCreateInfos(queueCreateInfos.data());
    deviceCreateInfo.setQueueCreateInfoCount(queueCreateInfos.size());
    deviceCreateInfo.setPEnabledFeatures(&deviceFeatures);
    deviceCreateInfo.setPpEnabledExtensionNames(deviceExtension.data());
    deviceCreateInfo.setEnabledExtensionCount(deviceExtension.size());

    if (enableValidationLayers) {
        deviceCreateInfo.setEnabledLayerCount(validationLayers.size());
        deviceCreateInfo.setPpEnabledLayerNames(validationLayers.data());
    }

    mDevice = mPhysicalDevice.createDevice(deviceCreateInfo);

    mPresentQueue = mDevice.getQueue(mIndices.presentFamily.value(), 0);
    mTransferQueue = mDevice.getQueue(mIndices.transferFamily.value(), 0);
    mGraphicsQueue = mDevice.getQueue(mIndices.graphicsFamily.value(), 0);
}

bool VulkanContext::checkValidationLayerSupport() {
    
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

std::vector<const char*> VulkanContext::getRequiredExtensions() {
    uint32_t glfwExtensionCount{0};
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void VulkanContext::createDescriptorPool() {
    uint32_t maxDescriptorSetUniformBuffers = 10000;
    uint32_t maxDescriptorSetSampledImages = 10000;
    uint32_t maxDescriptorSetDynamicUniformBuffers = 10000;

    std::vector<VkDescriptorPoolSize> poolSizes = {
        {
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            maxDescriptorSetUniformBuffers
        },
        {
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            maxDescriptorSetSampledImages
        },
        {
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            maxDescriptorSetDynamicUniformBuffers
        },
    };
    mDescriptorPool.setPoolSizes(poolSizes);
    // TODO: remove this magical constant
    mDescriptorPool.setMaxSets(100000);
    mDescriptorPool.create(mDevice);
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::debugCallback(
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