#ifndef COMMANDS
#define COMMANDS

#include <vector>

#include <vulkan/vulkan.h>

#include "vulkan/CommandPool.hpp"

class Commands {
    public:
        static VkCommandBuffer beginSingleTime(VkDevice device, CommandPool& commandPool);
        static void endSingleTime(VkDevice device,
                                  CommandPool& commandPool,
                                  VkCommandBuffer commandBuffer,
                                  VkQueue queue);
        static void allocateBuffers(VkDevice device,
                                    CommandPool& commandPool,
                                    std::vector<VkCommandBuffer>& buffers);
        static void begin(VkCommandBuffer buffer, VkCommandBufferUsageFlags flags);
};

#endif