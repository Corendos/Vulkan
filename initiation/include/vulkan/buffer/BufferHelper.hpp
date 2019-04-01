#ifndef BUFFERHELPER
#define BUFFERHELPER

#include <vulkan/vulkan.h>

#include "memory/MemoryManager.hpp"
#include "vulkan/CommandPool.hpp"
#include "vulkan/VulkanContext.hpp"

class BufferHelper {
    public:
        static void createBuffer(VulkanContext& context,
                                 VkDeviceSize size,
                                 VkBufferUsageFlags usage,
                                VkSharingMode sharingMode,
                                 VkMemoryPropertyFlags properties,
                                 VkBuffer& buffer,
                                 std::string name);
        static void copyBuffer(VulkanContext& context,
                               CommandPool& commandPool,
                               VkQueue queue,
                               VkBuffer srcBuffer,
                               VkBuffer dstBuffer,
                               VkDeviceSize size);
        static void copyBufferToImage(VulkanContext& context,
                                      CommandPool& commandPool,
                                      VkQueue queue,
                                      VkBuffer buffer,
                                      VkImage image,
                                      uint32_t width,
                                      uint32_t height);
};

#endif