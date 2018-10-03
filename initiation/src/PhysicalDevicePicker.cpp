#include <algorithm>
#include <iostream>

#include "PhysicalDevicePicker.hpp"


#pragma region PhysicalDevicePicker
PhysicalDevicePicker::PhysicalDevicePicker(VkInstance vulkanInstance) :
    mVulkanInstance(vulkanInstance) {}

PhysicalDevicePicker::PhysicalDevicePicker(VkInstance vulkanInstance, BasicLogger& logger) :
    mVulkanInstance(vulkanInstance), mOutLogger(&logger) {}

std::optional<VkPhysicalDevice> PhysicalDevicePicker::pick() {
    std::optional<VkPhysicalDevice> chosenPhysicalDevice;

    std::vector<VkPhysicalDevice> physicalDevices = retrieveDevices();

    listDevicesInfo(physicalDevices);

    if(!physicalDevices.empty()){
        chosenPhysicalDevice = *std::max_element(
            physicalDevices.begin(),
            physicalDevices.end(),
            PhysicalDeviceCompare());
    }
    
    return chosenPhysicalDevice;
}

std::vector<VkPhysicalDevice> PhysicalDevicePicker::retrieveDevices() {
    uint32_t deviceCount{0};
    vkEnumeratePhysicalDevices(mVulkanInstance, &deviceCount, nullptr);

    if (mOutLogger.has_value())
        *mOutLogger.value() << "[PhysicalDevicePicker] Found " << deviceCount << " device(s)" << std::endl;

    std::vector<VkPhysicalDevice> foundPhysicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(mVulkanInstance, &deviceCount, foundPhysicalDevices.data());

    return foundPhysicalDevices;
}

void PhysicalDevicePicker::listDevicesInfo(std::vector<VkPhysicalDevice>& physicalDevices) {
    if (!mOutLogger.has_value())
        return;

    *mOutLogger.value() << std::string(15, '=') << std::endl
        << " Device(s) information" << std::endl
        << std::string(15, '=') << std::endl;

    for (const auto& device : physicalDevices) {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(device, &properties);
        *mOutLogger.value() << "\t" << "Device Name: " << properties.deviceName << std::endl;
        *mOutLogger.value() << "\t" << "Device Type: " << properties.deviceType << std::endl;
        *mOutLogger.value() << "\t" << "Device ID: " << properties.deviceID << std::endl;
        *mOutLogger.value() << "\t" << "Driver version: " << properties.driverVersion << std::endl;
        *mOutLogger.value() << "\t" << "Vendor ID: " << properties.vendorID << std::endl;
    }
}

#pragma endregion

#pragma region PhysicalDeviceCompare
bool PhysicalDeviceCompare::operator()(const VkPhysicalDevice& a, const VkPhysicalDevice& b) {
    return PhysicalDeviceScorer::score(a) < PhysicalDeviceScorer::score(b);
}
#pragma endregion

#pragma region PhysicalDeviceScorer
uint32_t PhysicalDeviceScorer::score(const VkPhysicalDevice& physicalDevice) {
    uint32_t score{0};

    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU) {
        score += 100;
    }

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
        score += 500;
    }

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }

    if (deviceFeatures.geometryShader) {
        score += 100;
    }

    return score;
}
#pragma endregion