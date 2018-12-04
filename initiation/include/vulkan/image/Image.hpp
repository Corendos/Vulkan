#ifndef IMAGE
#define IMAGE

#include <exception>

#include <vulkan/vulkan.h>

#include "memory/MemoryManager.hpp"
#include "vulkan/CommandPool.hpp"
#include "vulkan/VulkanContext.hpp"

class Image {
    public:
        Image();
        void create(VulkanContext& context);
        void destroy(VulkanContext& context);

        void setImageType(VkImageType imageType);
        void setExtent(VkExtent3D extent);
        void setMipLevels(uint32_t mipLevels);
        void setArrayLayers(uint32_t arrayLayers);
        void setFormat(VkFormat format);
        void setTiling(VkImageTiling tiling);
        void setInitialLayout(VkImageLayout initialLayout);
        void setUsage(VkImageUsageFlags usage);
        void setSharingMode(VkSharingMode sharingMode);
        void setSamples(VkSampleCountFlagBits samples);
        void setFlags(VkFlags flags);

        VkImage getHandler();

    private:
        void _createImage(VkDevice device, MemoryManager& manager);

        VkImage mImage;
        VkImageCreateInfo mInfo{};
        bool mCreated{false};
};

#endif