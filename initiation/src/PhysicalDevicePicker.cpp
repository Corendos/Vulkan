#include <algorithm>
#include <iostream>

#include "PhysicalDevicePicker.hpp"


#pragma region PhysicalDevicePicker
PhysicalDevicePicker::PhysicalDevicePicker(VkInstance vulkanInstance, VkSurfaceKHR surface) :
    mVulkanInstance(vulkanInstance), mSurface(surface) {}

std::optional<PhysicalDeviceChoice> PhysicalDevicePicker::pick() {
    PhysicalDeviceInfo deviceInfo;
    std::optional<PhysicalDeviceChoice> deviceChoice;

    std::vector<PhysicalDeviceInfo> physicalDevices = getSuitableDevices();

    if(!physicalDevices.empty()){
        deviceInfo = *std::max_element(
            physicalDevices.begin(),
            physicalDevices.end(),
            PhysicalDeviceCompare());
        
        deviceChoice = {deviceInfo.device, deviceInfo.queueFamilyIndices};
    }
    
    return deviceChoice;
}

std::vector<PhysicalDeviceInfo> PhysicalDevicePicker::getSuitableDevices() {
    std::vector<PhysicalDeviceInfo> physicalDevices = retrieveDevices();

    auto end = std::remove_if(physicalDevices.begin(), physicalDevices.end(),
        [](PhysicalDeviceInfo& info) -> bool {
            return !info.queueFamilyIndices.isComplete();
    });

    physicalDevices.erase(end, physicalDevices.end());

    return physicalDevices;
}

std::vector<PhysicalDeviceInfo> PhysicalDevicePicker::retrieveDevices() {
    uint32_t deviceCount{0};
    vkEnumeratePhysicalDevices(mVulkanInstance, &deviceCount, nullptr);

    std::vector<VkPhysicalDevice> foundPhysicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(mVulkanInstance, &deviceCount, foundPhysicalDevices.data());

    std::vector<PhysicalDeviceInfo> devicesInfo(foundPhysicalDevices.size());

    for (size_t i{0}; i < devicesInfo.size();++i) {
        devicesInfo[i].device = foundPhysicalDevices[i];
        devicesInfo[i].properties = getDeviceProperties(devicesInfo[i].device);
        devicesInfo[i].features = getDeviceFeatures(devicesInfo[i].device);
        devicesInfo[i].queueFamilyIndices = findQueueFamily(devicesInfo[i].device);
    }

    return devicesInfo;
}

inline VkPhysicalDeviceProperties PhysicalDevicePicker::getDeviceProperties(VkPhysicalDevice& physicalDevice) {
    VkPhysicalDeviceProperties properties{};

    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    return properties;
}

inline VkPhysicalDeviceFeatures PhysicalDevicePicker::getDeviceFeatures(VkPhysicalDevice& physicalDevice) {
    VkPhysicalDeviceFeatures features{};

    vkGetPhysicalDeviceFeatures(physicalDevice, &features);

    return features;
}

QueueFamilyIndices PhysicalDevicePicker::findQueueFamily(VkPhysicalDevice& physicalDevice) {
    uint32_t queueFamilyCount{0};
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    QueueFamilyIndices queueFamilyIndices;

    uint32_t index{0};
    for (const auto& properties : queueFamilyProperties) {
        if (properties.queueCount > 0 && properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queueFamilyIndices.graphicsFamily = index;
        }

        VkBool32 presentSupport;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, index, mSurface, &presentSupport);

        if (properties.queueCount > 0 & presentSupport) {
            queueFamilyIndices.presentFamily = index;
        }

        if (queueFamilyIndices.isComplete()) {
            break;
        }

        index++;
    }

    return queueFamilyIndices;
}

#pragma endregion

#pragma region PhysicalDeviceCompare
bool PhysicalDeviceCompare::operator()(const PhysicalDeviceInfo& a, const PhysicalDeviceInfo& b) {
    return PhysicalDeviceScorer::score(a) < PhysicalDeviceScorer::score(b);
}
#pragma endregion

#pragma region PhysicalDeviceScorer
uint32_t PhysicalDeviceScorer::score(const PhysicalDeviceInfo& deviceInfo) {
    uint32_t score{0};

    if (deviceInfo.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU) {
        score += 100;
    }

    if (deviceInfo.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
        score += 500;
    }

    if (deviceInfo.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }

    if (deviceInfo.features.geometryShader) {
        score += 100;
    }

    return score;
}
#pragma endregion