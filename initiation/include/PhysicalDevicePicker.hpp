#ifndef PHYSICAL_DEVICE_PICKER
#define PHYSICAL_DEVICE_PICKER

#include <optional>
#include <vector>

#include <vulkan/vulkan.h>

#include <BasicLogger.hpp>

class PhysicalDevicePicker {
    public:
        PhysicalDevicePicker(VkInstance instance);
        PhysicalDevicePicker(VkInstance instance, BasicLogger& outLogger);
        std::optional<VkPhysicalDevice> pick();
    
    private:
        VkInstance mVulkanInstance;
        std::optional<BasicLogger*> mOutLogger;

        std::vector<VkPhysicalDevice> retrieveDevices();
        void listDevicesInfo(std::vector<VkPhysicalDevice>& physicalDevices);
};

class PhysicalDeviceCompare {
    public:
        bool operator()(const VkPhysicalDevice& a, const VkPhysicalDevice& b);
};

class PhysicalDeviceScorer {
    public:
        static uint32_t score(const VkPhysicalDevice& physicalDevice);
};

#endif