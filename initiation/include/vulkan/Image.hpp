#ifndef IMAGE
#define IMAGE

#include <exception>

#include <vulkan/vulkan.h>

#include "memory/MemoryManager.hpp"

class Image {
    public:
        static void create(VkDevice device, MemoryManager& manager,
                           uint32_t width, uint32_t height,
                           VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                           VkMemoryPropertyFlags properties, VkImage& image);
};

#endif