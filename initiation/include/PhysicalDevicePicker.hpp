#ifndef PHYSICAL_DEVICE_PICKER
#define PHYSICAL_DEVICE_PICKER

#include <optional>
#include <vector>

#include <vulkan/vulkan.h>

#include <BasicLogger.hpp>

#include "utils.hpp"

struct PhysicalDeviceChoice {
    VkPhysicalDevice physicalDevice;
    QueueFamilyIndices queueFamilyIndices;
};

struct PhysicalDeviceInfo {
    VkPhysicalDevice device;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    QueueFamilyIndices queueFamilyIndices;
};

class PhysicalDevicePicker {
    public:
        PhysicalDevicePicker(VkInstance instance, VkSurfaceKHR surface);
        std::optional<PhysicalDeviceChoice> pick();
    
    private:
        VkInstance mVulkanInstance;
        VkSurfaceKHR mSurface;

        std::vector<PhysicalDeviceInfo> retrieveDevices();

        std::vector<PhysicalDeviceInfo> getSuitableDevices();
        VkPhysicalDeviceProperties getDeviceProperties(VkPhysicalDevice& physicalDevice);
        VkPhysicalDeviceFeatures getDeviceFeatures(VkPhysicalDevice& physicalDevice);
        QueueFamilyIndices findQueueFamily(VkPhysicalDevice& physicalDevice);
};

class PhysicalDeviceCompare {
    public:
        bool operator()(const PhysicalDeviceInfo& a, const PhysicalDeviceInfo& b);
};

class PhysicalDeviceScorer {
    public:
        static uint32_t score(const PhysicalDeviceInfo& deviceInfo);
};

#endif