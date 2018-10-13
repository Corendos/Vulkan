#ifndef MEMORYMANAGER
#define MEMORYMANAGER

#include <vector>
#include <map>

#include <vulkan/vulkan.h>

#include "MemoryBlock.hpp"
#include "BufferInfo.hpp"

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
        void freeBuffer(VkBuffer& buffer);
        void mapMemory(VkBuffer& buffer, VkDeviceSize size, void** data);
        void unmapMemory(VkBuffer& buffer);

        void memoryCheckLog();

    private:
        VkDevice& mDevice;
        VkPhysicalDevice& mPhysicalDevice;
        VkPhysicalDeviceMemoryProperties mMemoryProperties;
        std::vector<VkDeviceMemory> mMemoryHeaps;

        std::vector<std::vector<MemoryBlock>> mMemoryOccupations;
        std::map<VkBuffer, BufferInfo> mBuffersInfo;
        static uint32_t initialAllocationSize;
        static uint32_t pageSize;

        void initialAllocation();
        int32_t findMemoryType(VkMemoryRequirements& memoryRequirements);
        int32_t findSuitableMemoryBlock(uint32_t blockCount, uint32_t memoryHeapIndex);

        static uint32_t getBlockCount(uint32_t requiredSize);
};

#endif