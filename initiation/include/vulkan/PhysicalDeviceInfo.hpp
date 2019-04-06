#ifndef PHYSICALDEVICEINFO
#define PHYSICALDEVICEINFO

#include <vulkan/vulkan.hpp>

#include "vulkan/QueueFamilyIndices.hpp"

struct PhysicalDeviceInfo {
    vk::PhysicalDevice device;
    vk::PhysicalDeviceProperties properties;
    vk::PhysicalDeviceFeatures features;
    vk::PhysicalDeviceLimits limits;
    QueueFamilyIndices queueFamilyIndices;
    std::vector<std::string> supportedExtensions;
};

#endif