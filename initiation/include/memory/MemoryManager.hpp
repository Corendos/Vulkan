#ifndef MEMORYMANAGER
#define MEMORYMANAGER

#include <vector>
#include <map>
#include <tuple>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include "MemoryBlock.hpp"
#include "MemoryHeapOccupation.hpp"
#include "BufferInfo.hpp"

#include "memory/Chunk.hpp"

class MemoryManager {
    public:
        MemoryManager() = delete;
        MemoryManager(MemoryManager& other) = delete;
        MemoryManager(MemoryManager&& other) = delete;
        MemoryManager(vk::PhysicalDevice& physicalDevice, vk::Device& device);

        void init();
        void printInfo();
        void cleanup();

        void allocateForBuffer(vk::Buffer buffer,
                               vk::MemoryRequirements& memoryRequirements,
                               vk::MemoryPropertyFlags properties,
                               std::string name);
        void allocateForImage(vk::Image image,
                              vk::MemoryRequirements& memoryRequirements,
                              vk::MemoryPropertyFlags properties,
                              std::string name);
        void freeBuffer(vk::Buffer buffer);
        void freeImage(vk::Image image);
        void mapMemory(vk::Buffer buffer, vk::DeviceSize size, void** data);
        void unmapMemory(vk::Buffer buffer);

        void memoryCheckLog();

    private:
        vk::Device& mDevice;
        vk::PhysicalDevice& mPhysicalDevice;
        vk::PhysicalDeviceMemoryProperties mMemoryProperties;
        static uint32_t allocationSize;
        static uint32_t pageSize;

        std::map<uint32_t, std::vector<Chunk>> mChunksMap;
        std::map<vk::Buffer, BufferInfo> mBuffersInfo;
        std::map<vk::Image, BufferInfo> mImagesInfo;

        void allocate(uint32_t memoryTypeIndex);
        int32_t findMemoryType(vk::MemoryRequirements& memoryRequirements, vk::MemoryPropertyFlags& properties);
};

#endif