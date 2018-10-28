#ifndef COMMANDS
#define COMMANDS

#include <vulkan/vulkan.h>

#include "vulkan/CommandPool.hpp"

class Commands {
    public:
        static VkCommandBuffer beginSingleTime(VkDevice device, CommandPool& commandPool);
        static void endSingleTime(VkDevice device,
                                  CommandPool& commandPool,
                                  VkCommandBuffer commandBuffer,
                                  VkQueue queue);
};

#endif