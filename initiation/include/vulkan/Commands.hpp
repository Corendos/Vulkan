#ifndef COMMANDS
#define COMMANDS

#include <vector>

#include <vulkan/vulkan.hpp>

class Commands {
    public:
        static vk::CommandBuffer beginSingleTime(vk::Device device, vk::CommandPool& commandPool);
        static void endSingleTime(vk::Device device,
                                  vk::CommandPool& commandPool,
                                  vk::CommandBuffer commandBuffer,
                                  vk::Queue queue);
};

#endif