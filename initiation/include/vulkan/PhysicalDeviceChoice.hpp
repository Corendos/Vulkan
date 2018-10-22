#ifndef PHYSICALDEVICECHOICE
#define PHYSICALDEVICECHOICE

#include <vulkan/vulkan.h>

#include "vulkan/QueueFamilyIndices.hpp"

struct PhysicalDeviceChoice {
    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    QueueFamilyIndices queueFamilyIndices;

    bool isComplete() {
        return queueFamilyIndices.isComplete() && (physicalDevice != VK_NULL_HANDLE);
    }
};

#endif