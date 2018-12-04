#ifndef IMAGEHELPER
#define IMAGEHELPER

#include <vulkan/vulkan.h>

#include "vulkan/VulkanContext.hpp"

class ImageHelper {
    public:
        static VkImage create(VulkanContext& context,
                           uint32_t width, uint32_t height,
                           VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                           VkMemoryPropertyFlags properties);
        static VkImageView createImageView(VulkanContext& context,
                                    VkImage image,
                                    VkFormat format,
                                    VkImageAspectFlags aspectFlags);
        static void transitionImageLayout(VulkanContext& context,
                                          VkImage image,
                                          VkFormat format,
                                          VkImageLayout oldLayout,
                                          VkImageLayout newLayout);
    private:
        static bool hasStencilComponent(VkFormat format);
};

#endif