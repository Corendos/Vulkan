#ifndef PHYSICALDEVICEINFO
#define PHYSICALDEVICEINFO

#include <vulkan/vulkan.h>

#include "vulkan/QueueFamilyIndices.hpp"

struct PhysicalDeviceInfo {
    VkPhysicalDevice device;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceLimits limits;
    QueueFamilyIndices queueFamilyIndices;
    std::vector<std::string> supportedExtensions;
};

#endif