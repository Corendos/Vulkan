#ifndef MEMORYMANAGER
#define MEMORYMANAGER

#include <vector>

#include <vulkan/vulkan.h>

class MemoryManager {
    public:
        MemoryManager() = delete;
        MemoryManager(MemoryManager& other) = delete;
        MemoryManager(MemoryManager&& other) = delete;
        MemoryManager(VkPhysicalDevice& physicalDevice, VkDevice& device);

        void init();
        void printInfo();
        void cleanup();

        void allocateForBuffer(VkBuffer& buffer, VkMemoryRequirements& memoryRequirements);

    private:
        VkDevice& mDevice;
        VkPhysicalDevice& mPhysicalDevice;
        VkPhysicalDeviceMemoryProperties mMemoryProperties;
        std::vector<VkDeviceMemory> mMemoryHeaps;

        std::vector<std::vector<bool>> mMemoryOccupations;
        static uint32_t initialAllocationSize;
        static uint32_t pageSize;

        void initialAllocation();
        static uint32_t getBlockCount(uint32_t requiredSize);
};

#endif