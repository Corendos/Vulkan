#ifndef PHYSICALDEVICECOMPARATOR
#define PHYSICALDEVICECOMPARATOR

#include "vulkan/PhysicalDeviceInfo.hpp"

class PhysicalDeviceComparator {
    public:
        bool operator()(const PhysicalDeviceInfo& a, const PhysicalDeviceInfo& b) {
            return scoreDevice(a) < scoreDevice(b);
        }

        static uint32_t scoreDevice(const PhysicalDeviceInfo& info) {
            uint32_t score{0};
            if (info.properties.deviceType == vk::PhysicalDeviceType::eCpu)
                score += 100;

            if (info.properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu)
                score += 500;

            if (info.properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
                score += 1000;

            if (info.features.geometryShader)
                score += 100;

            return score;
        }
};

#endif