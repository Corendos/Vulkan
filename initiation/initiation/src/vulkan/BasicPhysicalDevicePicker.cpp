#include "vulkan/BasicPhysicalDevicePicker.hpp"
#include "vulkan/PhysicalDeviceComparator.hpp"

BasicPhysicalDevicePicker::BasicPhysicalDevicePicker(vk::Instance instance, vk::SurfaceKHR surface, std::vector<const char*> requiredExtensions)
    : mInstance(instance), mSurface(surface) {
    mRequiredExtensions = std::vector<std::string>(requiredExtensions.size());
    std::transform(
        requiredExtensions.begin(), requiredExtensions.end(),
        mRequiredExtensions.begin(), [](const char* name) { return std::string(name); });
    std::sort(mRequiredExtensions.begin(), mRequiredExtensions.end());
}

PhysicalDeviceChoice BasicPhysicalDevicePicker::pick() {
    PhysicalDeviceChoice choice;
    std::vector<PhysicalDeviceInfo> deviceList = getDeviceInfoList();

    using namespace std::placeholders;

    auto endIt = std::remove_if(deviceList.begin(), deviceList.end(),
        std::bind(isPhysicalDeviceNotSuitable, _1, *this));
    deviceList.erase(endIt, deviceList.end());
    auto bestPhysicalDeviceInfo = std::max_element(
        deviceList.begin(), deviceList.end(), PhysicalDeviceComparator());

    if (bestPhysicalDeviceInfo != deviceList.end()) {
        choice = {
            bestPhysicalDeviceInfo->device,
            bestPhysicalDeviceInfo->properties.limits,
            bestPhysicalDeviceInfo->queueFamilyIndices
        };
    }

    return choice;
}

std::vector<PhysicalDeviceInfo> BasicPhysicalDevicePicker::getDeviceInfoList() {
    std::vector<vk::PhysicalDevice> foundPhysicalDevices = mInstance.enumeratePhysicalDevices();

    std::vector<PhysicalDeviceInfo> devicesInfo(foundPhysicalDevices.size());

    for (size_t i{0}; i < devicesInfo.size();++i) {
        devicesInfo[i].device = foundPhysicalDevices[i];
        devicesInfo[i].properties = getDeviceProperties(devicesInfo[i].device);
        devicesInfo[i].features = getDeviceFeatures(devicesInfo[i].device);
        devicesInfo[i].queueFamilyIndices = getFamiliesIndices(devicesInfo[i].device);
        devicesInfo[i].supportedExtensions = getSupportedExtensions(devicesInfo[i].device);
    }

    return devicesInfo;
}

vk::PhysicalDeviceProperties BasicPhysicalDevicePicker::getDeviceProperties(vk::PhysicalDevice physicalDevice) {
    vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
    return properties;
}

vk::PhysicalDeviceFeatures BasicPhysicalDevicePicker::getDeviceFeatures(vk::PhysicalDevice physicalDevice) {
    vk::PhysicalDeviceFeatures features = physicalDevice.getFeatures();
    return features;
}

QueueFamilyIndices BasicPhysicalDevicePicker::getFamiliesIndices(vk::PhysicalDevice physicalDevice) {
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

    QueueFamilyIndices queueFamilyIndices;

    uint32_t index{0};
    for (const auto& properties : queueFamilyProperties) {
        if (hasGraphicsSupport(properties))
            queueFamilyIndices.graphicsFamily = index;

        if (hasPresentSupport(properties, physicalDevice, index))
            queueFamilyIndices.presentFamily = index;

        if (hasTransferSupport(properties))
            queueFamilyIndices.transferFamily = index;

        if (queueFamilyIndices.isComplete())
            break;
        
        index++;
    }

    return queueFamilyIndices;
}

std::vector<std::string> BasicPhysicalDevicePicker::getSupportedExtensions(vk::PhysicalDevice physicalDevice) {
    std::vector<vk::ExtensionProperties> availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();

    std::vector<std::string> availableExtensionsName(availableExtensions.size());
    std::transform(
        availableExtensions.begin(), availableExtensions.end(),
        availableExtensionsName.begin(), [](vk::ExtensionProperties& prop) { return prop.extensionName; });

    std::sort(availableExtensionsName.begin(), availableExtensionsName.end());

    return availableExtensionsName;
}

bool BasicPhysicalDevicePicker::hasGraphicsSupport(vk::QueueFamilyProperties properties) {
    return properties.queueCount > 0 && properties.queueFlags & vk::QueueFlagBits::eGraphics;
}

bool BasicPhysicalDevicePicker::hasPresentSupport(vk::QueueFamilyProperties properties, vk::PhysicalDevice physicalDevice, uint32_t queueFamilyIndex) {
    return properties.queueCount > 0 && physicalDevice.getSurfaceSupportKHR(queueFamilyIndex, mSurface);
}

bool BasicPhysicalDevicePicker::hasTransferSupport(vk::QueueFamilyProperties properties) {
    return properties.queueCount > 0 && properties.queueFlags & vk::QueueFlagBits::eTransfer;
}

bool BasicPhysicalDevicePicker::isPhysicalDeviceNotSuitable(PhysicalDeviceInfo info, BasicPhysicalDevicePicker& picker) {
    bool extensionSupported = std::includes(
        info.supportedExtensions.begin(), info.supportedExtensions.end(),
        picker.mRequiredExtensions.begin(), picker.mRequiredExtensions.end());
    return !(info.queueFamilyIndices.isComplete() && info.features.samplerAnisotropy && extensionSupported);
}