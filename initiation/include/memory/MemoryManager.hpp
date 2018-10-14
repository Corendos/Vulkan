#ifndef MEMORYMANAGER
#define MEMORYMANAGER

#include <vector>
#include <map>
#include <tuple>

#include <vulkan/vulkan.h>

#include "MemoryBlock.hpp"
#include "MemoryHeapOccupation.hpp"
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

        void allocateForBuffer(VkBuffer& buffer, VkMemoryRequirements& memoryRequirements, VkMemoryPropertyFlags properties);
        void allocateForImage(VkImage& image, VkMemoryRequirements& memoryRequirements, VkMemoryPropertyFlags properties);
        void freeBuffer(VkBuffer& buffer);
        void freeImage(VkImage& image);
        void mapMemory(VkBuffer& buffer, VkDeviceSize size, void** data);
        void unmapMemory(VkBuffer& buffer);

        void memoryCheckLog();

    private:
        VkDevice& mDevice;
        VkPhysicalDevice& mPhysicalDevice;
        VkPhysicalDeviceMemoryProperties mMemoryProperties;
        std::vector<std::vector<VkDeviceMemory>> mDeviceMemoryAllocation;

        std::vector<std::vector<MemoryHeapOccupation>> mMemoryHeapOccupations;
        std::map<VkBuffer, BufferInfo> mBuffersInfo;
        std::map<VkImage, BufferInfo> mImagesInfo;
        static uint32_t allocationSize;
        static uint32_t pageSize;

        void initialAllocation();
        void allocate(uint32_t memoryTypeIndex);
        int32_t findMemoryType(VkMemoryRequirements& memoryRequirements, VkMemoryPropertyFlags& properties);
        std::tuple<int32_t, int32_t> findSuitableMemoryBlock(uint32_t blockCount, uint32_t memoryHeapIndex);
        int32_t findSuitableMemoryBlock(uint32_t blockCount, uint32_t memoryHeapIndex, uint32_t memoryHeapOffset);
        static uint32_t getBlockCount(uint32_t requiredSize);
};

#endif