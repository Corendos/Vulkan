#ifndef IMAGEHELPER
#define IMAGEHELPER

#include <vulkan/vulkan.h>

#include "vulkan/VulkanContext.hpp"

class ImageHelper {
    public:
        static vk::Image create(VulkanContext& context,
                           uint32_t width, uint32_t height,
                           vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                           vk::MemoryPropertyFlags properties);
        static void transitionImageLayout(VulkanContext& context,
                                          vk::Image image,
                                          vk::Format format,
                                          vk::ImageLayout oldLayout,
                                          vk::ImageLayout newLayout);
    private:
        static bool hasStencilComponent(vk::Format format);
};

#endif