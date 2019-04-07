#ifndef PHYSICALDEVICECHOICE
#define PHYSICALDEVICECHOICE

#include <vulkan/vulkan.hpp>

#include "vulkan/QueueFamilyIndices.hpp"

struct PhysicalDeviceChoice {
    vk::PhysicalDevice physicalDevice;
    vk::PhysicalDeviceLimits limits;
    QueueFamilyIndices queueFamilyIndices;

    bool isComplete() {
        return queueFamilyIndices.isComplete()
            && (physicalDevice != vk::PhysicalDevice());
    }
};

#endif