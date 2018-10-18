#ifndef BASICPHYSICALDEVICEPICKER
#define BASICPHYSICALDEVICEPICKER

#include <vector>
#include <algorithm>
#include <functional>

#include <vulkan/vulkan.h>

#include "device/PhysicalDeviceInfo.hpp"
#include "device/PhysicalDeviceChoice.hpp"
#include "device/QueueFamilyIndices.hpp"
#include "device/PhysicalDeviceComparator.hpp"

class BasicPhysicalDevicePicker {
    public:
        BasicPhysicalDevicePicker(VkInstance instance, VkSurfaceKHR surface, std::vector<const char*> requiredExtensions);
        PhysicalDeviceChoice pick();

    private:
        std::vector<PhysicalDeviceInfo> getDeviceInfoList();
        VkPhysicalDeviceProperties getDeviceProperties(VkPhysicalDevice physicalDevice);
        VkPhysicalDeviceFeatures getDeviceFeatures(VkPhysicalDevice physicalDevice);
        QueueFamilyIndices getFamiliesIndices(VkPhysicalDevice physicalDevice);
        std::vector<std::string> getSupportedExtensions(VkPhysicalDevice physicalDevice);

        bool hasGraphicsSupport(VkQueueFamilyProperties properties);
        bool hasPresentSupport(VkQueueFamilyProperties properties, VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);

        static bool isPhysicalDeviceNotSuitable(PhysicalDeviceInfo info, BasicPhysicalDevicePicker& picker);

        VkInstance mInstance;
        VkSurfaceKHR mSurface;
        std::vector<std::string> mRequiredExtensions;
};

#endif