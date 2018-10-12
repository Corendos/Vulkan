#ifndef MEMORYMANAGER
#define MEMORYMANAGER

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

    private:
        VkDevice& mDevice;
        VkPhysicalDevice& mPhysicalDevice;
        VkDeviceMemory mMemory;
        uint32_t mMemoryHeapIndex;
        uint32_t mMemoryTypeIndex;

        void findSuitableMemoryHeap();
        void initialAllocation();
};

#endif