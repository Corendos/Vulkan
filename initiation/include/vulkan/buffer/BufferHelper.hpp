#ifndef BUFFERHELPER
#define BUFFERHELPER

#include <vulkan/vulkan.hpp>

#include "memory/MemoryManager.hpp"
#include "vulkan/VulkanContext.hpp"

class BufferHelper {
    public:
        static void createBuffer(VulkanContext& context,
                                 vk::DeviceSize size,
                                 vk::BufferUsageFlags usage,
                                 vk::SharingMode sharingMode,
                                 vk::MemoryPropertyFlags properties,
                                 vk::Buffer& buffer,
                                 std::string name);
        static void copyBuffer(VulkanContext& context,
                               vk::CommandPool& commandPool,
                               vk::Queue queue,
                               vk::Buffer srcBuffer,
                               vk::Buffer dstBuffer,
                               vk::DeviceSize size);
        static void copyBufferToImage(VulkanContext& context,
                                      vk::CommandPool& commandPool,
                                      vk::Queue queue,
                                      vk::Buffer buffer,
                                      vk::Image image,
                                      uint32_t width,
                                      uint32_t height);
};

#endif