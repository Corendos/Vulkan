#ifndef MEMORYMANAGER
#define MEMORYMANAGER

#include <vector>
#include <map>
#include <tuple>

#include <vulkan/vulkan.h>

#include "MemoryBlock.hpp"
#include "MemoryHeapOccupation.hpp"
#include "BufferInfo.hpp"

#include "memory/Chunk.hpp"

class MemoryManager {
    public:
        MemoryManager() = delete;
        MemoryManager(MemoryManager& other) = delete;
        MemoryManager(MemoryManager&& other) = delete;
        MemoryManager(VkPhysicalDevice& physicalDevice, VkDevice& device);

        void init();
        void printInfo();
        void cleanup();

        void allocateForBuffer(VkBuffer buffer,
                               VkMemoryRequirements& memoryRequirements,
                               VkMemoryPropertyFlags properties,
                               std::string name);
        void allocateForImage(VkImage image,
                              VkMemoryRequirements& memoryRequirements,
                              VkMemoryPropertyFlags properties,
                              std::string name);
        void freeBuffer(VkBuffer buffer);
        void freeImage(VkImage image);
        void mapMemory(VkBuffer buffer, VkDeviceSize size, void** data);
        void unmapMemory(VkBuffer buffer);

        void memoryCheckLog();

    private:
        VkDevice& mDevice;
        VkPhysicalDevice& mPhysicalDevice;
        VkPhysicalDeviceMemoryProperties mMemoryProperties;
        static uint32_t allocationSize;
        static uint32_t pageSize;

        std::map<uint32_t, std::vector<Chunk>> mChunksMap;
        std::map<VkBuffer, BufferInfo> mBuffersInfo;
        std::map<VkImage, BufferInfo> mImagesInfo;

        void allocate(uint32_t memoryTypeIndex);
        int32_t findMemoryType(VkMemoryRequirements& memoryRequirements, VkMemoryPropertyFlags& properties);
};

#endif