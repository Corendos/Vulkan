#ifndef IMAGE
#define IMAGE

#include <exception>

#include <vulkan/vulkan.h>

#include "memory/MemoryManager.hpp"
#include "vulkan/CommandPool.hpp"
#include "vulkan/VulkanContext.hpp"

class Image {
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

        void create(VulkanContext& context);
        void destroy(VulkanContext& context);

        void loadFromFile(const std::string filename, VulkanContext& context);

        VkImage getHandler() const;
        VkImageView getViewHandler() const;
        VkSampler getSamplerHandler() const;

    private:
        static bool hasStencilComponent(VkFormat format);
        void _createImage(VkDevice device, MemoryManager& manager);
        void _createImageView(VkDevice device);
        void _createImageSampler(VkDevice device);

        VkImage mImage;
        VkImageView mImageView;
        VkSampler mSampler;

        VkBuffer mStagingBuffer;

        uint32_t mWidth;
        uint32_t mHeight;
        uint32_t mBpp;

        bool mLoaded{false};
        bool mCreated{false};
};

#endif