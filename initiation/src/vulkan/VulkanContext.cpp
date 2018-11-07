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
    mGraphicsCommandPool.create(mDevice, mIndices.graphicsFamily.value());
    mTransferCommandPool.create(mDevice, mIndices.transferFamily.value());
    mMemoryManager.init();
}

void VulkanContext::destroy() {
    vkDeviceWaitIdle(mDevice);

    if (enableValidationLayers) {
        destroyDebugUtilsMessengerEXT(mInstance, mCallback, nullptr);
    }

    mMemoryManager.cleanup();
    mMemoryManager.memoryCheckLog();
    mGraphicsCommandPool.destroy(mDevice);
    mTransferCommandPool.destroy(mDevice);
    vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
    vkDestroyDevice(mDevice, nullptr);
    vkDestroyInstance(mInstance, nullptr);
}

VkInstance VulkanContext::getInstance() const {
    return mInstance;
}

VkPhysicalDevice VulkanContext::getPhysicalDevice() const {
    return mPhysicalDevice;
}

VkDevice VulkanContext::getDevice() const {
    return mDevice;
}

VkPhysicalDeviceLimits VulkanContext::getLimits() const {
    return mPhysicalDeviceLimits;
}

VkQueue VulkanContext::getGraphicsQueue() const {
    return mGraphicsQueue;
}

VkQueue VulkanContext::getPresentQueue() const {
    return mPresentQueue;
}

VkQueue VulkanContext::getTransferQueue() const {
    return mTransferQueue;
}

QueueFamilyIndices VulkanContext::getQueueFamilyIndices() const {
    return mIndices;
}

CommandPool& VulkanContext::getGraphicsCommandPool() {
    return mGraphicsCommandPool;
}

CommandPool& VulkanContext::getTransferCommandPool() {
    return mTransferCommandPool;
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

void VulkanContext::createInstance() {
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
}

void VulkanContext::createLogicalDevice() {
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
    vkGetDeviceQueue(mDevice, mIndices.transferFamily.value(), 0, &mTransferQueue);
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