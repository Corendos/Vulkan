#ifndef BASICPHYSICALDEVICEPICKER
#define BASICPHYSICALDEVICEPICKER

#include <vector>
#include <algorithm>
#include <functional>

#include <vulkan/vulkan.hpp>

#include "vulkan/PhysicalDeviceInfo.hpp"
#include "vulkan/PhysicalDeviceChoice.hpp"
#include "vulkan/QueueFamilyIndices.hpp"

class BasicPhysicalDevicePicker {
    public:
        BasicPhysicalDevicePicker(vk::Instance instance, vk::SurfaceKHR surface, std::vector<const char*> requiredExtensions);
        PhysicalDeviceChoice pick();

    private:
        std::vector<PhysicalDeviceInfo> getDeviceInfoList();
        vk::PhysicalDeviceProperties getDeviceProperties(vk::PhysicalDevice physicalDevice);
        vk::PhysicalDeviceFeatures getDeviceFeatures(vk::PhysicalDevice physicalDevice);
        QueueFamilyIndices getFamiliesIndices(vk::PhysicalDevice physicalDevice);
        std::vector<std::string> getSupportedExtensions(vk::PhysicalDevice physicalDevice);

        bool hasGraphicsSupport(vk::QueueFamilyProperties properties);
        bool hasTransferSupport(vk::QueueFamilyProperties properties);
        bool hasPresentSupport(vk::QueueFamilyProperties properties, vk::PhysicalDevice physicalDevice, uint32_t queueFamilyIndex);

        static bool isPhysicalDeviceNotSuitable(PhysicalDeviceInfo info, BasicPhysicalDevicePicker& picker);

        vk::Instance mInstance;
        vk::SurfaceKHR mSurface;
        std::vector<std::string> mRequiredExtensions;
};

#endif