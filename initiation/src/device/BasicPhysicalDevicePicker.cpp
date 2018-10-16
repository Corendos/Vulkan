#include "device/BasicPhysicalDevicePicker.hpp"

BasicPhysicalDevicePicker::BasicPhysicalDevicePicker(VkInstance instance, VkSurfaceKHR surface)
    : mInstance(instance), mSurface(surface) {}

PhysicalDeviceChoice BasicPhysicalDevicePicker::pick() {
    PhysicalDeviceChoice choice;
    std::vector<PhysicalDeviceInfo> deviceList = getDeviceInfoList();

    auto endIt = std::remove_if(deviceList.begin(), deviceList.end(), isPhysicalDeviceNotSuitable);
    deviceList.erase(endIt, deviceList.end());
    auto bestPhysicalDeviceInfo = std::max_element(
        deviceList.begin(), deviceList.end(), PhysicalDeviceComparator());

    if (bestPhysicalDeviceInfo != deviceList.end()) {
        choice = {bestPhysicalDeviceInfo->device, bestPhysicalDeviceInfo->queueFamilyIndices};
    }

    return choice;
}

std::vector<PhysicalDeviceInfo> BasicPhysicalDevicePicker::getDeviceInfoList() {
    uint32_t deviceCount{0};
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

    std::vector<VkPhysicalDevice> foundPhysicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, foundPhysicalDevices.data());

    std::vector<PhysicalDeviceInfo> devicesInfo(foundPhysicalDevices.size());

    for (size_t i{0}; i < devicesInfo.size();++i) {
        devicesInfo[i].device = foundPhysicalDevices[i];
        devicesInfo[i].properties = getDeviceProperties(devicesInfo[i].device);
        devicesInfo[i].features = getDeviceFeatures(devicesInfo[i].device);
        devicesInfo[i].queueFamilyIndices = getFamiliesIndices(devicesInfo[i].device);
    }

    return devicesInfo;
}

VkPhysicalDeviceProperties BasicPhysicalDevicePicker::getDeviceProperties(VkPhysicalDevice physicalDevice) {
    VkPhysicalDeviceProperties properties{};

    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    return properties;
}

VkPhysicalDeviceFeatures BasicPhysicalDevicePicker::getDeviceFeatures(VkPhysicalDevice physicalDevice) {
    VkPhysicalDeviceFeatures features{};

    vkGetPhysicalDeviceFeatures(physicalDevice, &features);

    return features;
}

QueueFamilyIndices BasicPhysicalDevicePicker::getFamiliesIndices(VkPhysicalDevice physicalDevice) {
    uint32_t queueFamilyCount{0};

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    QueueFamilyIndices queueFamilyIndices;

    uint32_t index{0};
    for (const auto& properties : queueFamilyProperties) {
        if (hasGraphicsSupport(properties))
            queueFamilyIndices.graphicsFamily = index;

        if (hasPresentSupport(properties, physicalDevice, index))
            queueFamilyIndices.presentFamily = index;

        if (queueFamilyIndices.isComplete())
            break;
        
        index++;
    }

    return queueFamilyIndices;
}

bool BasicPhysicalDevicePicker::hasGraphicsSupport(VkQueueFamilyProperties properties) {
    return properties.queueCount > 0 && properties.queueFlags & VK_QUEUE_GRAPHICS_BIT;
}

bool BasicPhysicalDevicePicker::hasPresentSupport(VkQueueFamilyProperties properties, VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex) {
    VkBool32 supported;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, mSurface, &supported);
    
    return properties.queueCount > 0 && supported;
}

bool BasicPhysicalDevicePicker::isPhysicalDeviceNotSuitable(PhysicalDeviceInfo info) {
    return !(info.queueFamilyIndices.isComplete() && info.features.samplerAnisotropy);
}