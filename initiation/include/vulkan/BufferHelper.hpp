#ifndef BUFFERHELPER
#define BUFFERHELPER

#include <vulkan/vulkan.h>

#include "memory/MemoryManager.hpp"
#include "vulkan/CommandPool.hpp"

class BufferHelper {
    public:
        static void createBuffer(MemoryManager& manager,
                                 VkDevice device,
                                 VkDeviceSize size,
                                 VkBufferUsageFlags usage,
                                 VkMemoryPropertyFlags properties,
                                 VkBuffer& buffer);
        static void copyBuffer(MemoryManager& manager,
                               VkDevice device,
                               CommandPool& commandPool,
                               VkQueue queue,
                               VkBuffer srcBuffer,
                               VkBuffer dstBuffer,
                               VkDeviceSize size);
        static void copyBufferToImage(VkDevice device,
                                      CommandPool& commandPool,
                                      VkQueue queue,
                                      VkBuffer buffer,
                                      VkImage image,
                                      uint32_t width,
                                      uint32_t height);
};

#endif