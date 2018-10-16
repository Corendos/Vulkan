#ifndef PHYSICALDEVICECOMPARATOR
#define PHYSICALDEVICECOMPARATOR

#include "device/PhysicalDeviceInfo.hpp"

class PhysicalDeviceComparator {
    public:
        bool operator()(const PhysicalDeviceInfo& a, const PhysicalDeviceInfo& b) {
            return scoreDevice(a) < scoreDevice(b);
        }

        static uint32_t scoreDevice(const PhysicalDeviceInfo& info) {
            uint32_t score{0};

            if (info.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
                score += 100;

            if (info.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
                score += 500;

            if (info.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                score += 1000;

            if (info.features.geometryShader)
                score += 100;

            return score;
        }
};

#endif