#ifndef COMMANDS
#define COMMANDS

#include <vector>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include "vulkan/CommandPool.hpp"

class Commands {
    public:
        static vk::CommandBuffer beginSingleTime(vk::Device device, CommandPool& commandPool);
        static void endSingleTime(vk::Device device,
                                  CommandPool& commandPool,
                                  vk::CommandBuffer commandBuffer,
                                  vk::Queue queue);
};

#endif