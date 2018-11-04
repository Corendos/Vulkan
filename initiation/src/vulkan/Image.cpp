#include "vulkan/Image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "vulkan/Commands.hpp"
#include "vulkan/BufferHelper.hpp"

VkImage Image::create(VulkanContext& context,
                   uint32_t width, uint32_t height,
                   VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties) {
    VkImage image;

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    if (vkCreateImage(context.getDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create image");
    }

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(context.getDevice(), image, &memoryRequirements);

    context.getMemoryManager().allocateForImage(image, memoryRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    return image;
}

VkImageView Image::createImageView(VulkanContext& context,
                                   VkImage image,
                                   VkFormat format,
                                   VkImageAspectFlags aspectFlags) {
    VkImageView imageView;

    VkImageViewCreateInfo imageViewInfo{};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = image;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format = format;
    imageViewInfo.subresourceRange.aspectMask = aspectFlags;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.levelCount = 1;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(context.getDevice(), &imageViewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image view");
    }

    return imageView;
}

void Image::transitionImageLayout(VulkanContext& context,
                                  VkImage image,
                                  VkFormat format,
                                  VkImageLayout oldLayout,
                                  VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = Commands::beginSingleTime(context.getDevice(), context.getCommandPool());

    VkImageMemoryBarrier memoryBarrier{};
    memoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    memoryBarrier.oldLayout = oldLayout;
    memoryBarrier.newLayout = newLayout;
    memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    memoryBarrier.image = image;
    memoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    memoryBarrier.subresourceRange.baseMipLevel = 0;
    memoryBarrier.subresourceRange.levelCount = 1;
    memoryBarrier.subresourceRange.baseArrayLayer = 0;
    memoryBarrier.subresourceRange.layerCount = 1;
    memoryBarrier.srcAccessMask = 0;
    memoryBarrier.dstAccessMask = 0;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        memoryBarrier.srcAccessMask = 0;
        memoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        memoryBarrier.srcAccessMask = 0;
        memoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else {
        throw std::runtime_error("Unsupported layout transition");
    }

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        memoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (hasStencilComponent(format)) {
            memoryBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else {
        memoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &memoryBarrier
    );

    Commands::endSingleTime(context.getDevice(),
                            context.getCommandPool(),
                            commandBuffer,
                            context.getGraphicsQueue());
}

bool Image::hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void Image::loadFromFile(const std::string filename, VulkanContext& context) {
    int width, height, bpp;
    uint8_t* pixels = stbi_load(filename.c_str(), &width, &height, &bpp, 4);

    mWidth = static_cast<uint32_t>(width);
    mHeight = static_cast<uint32_t>(height);
    mBpp = static_cast<uint32_t>(bpp);
    
    VkDeviceSize size = mWidth * mHeight * 4;
    BufferHelper::createBuffer(context.getMemoryManager(),
                               context.getDevice(), size,
                               VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                               mStagingBuffer);

    void* data;
    context.getMemoryManager().mapMemory(mStagingBuffer, size, &data);
    memcpy(data, pixels, size);
    context.getMemoryManager().unmapMemory(mStagingBuffer);

    stbi_image_free(pixels);
    mLoaded = true;
}

void Image::create(VulkanContext& context) {
    _createImage(context.getDevice(), context.getMemoryManager());
    Image::transitionImageLayout(context, mImage, VK_FORMAT_R8G8B8A8_UNORM,
                          VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    BufferHelper::copyBufferToImage(context.getDevice(),
                                    context.getCommandPool(),
                                    context.getGraphicsQueue(),
                                    mStagingBuffer, mImage,
                                    mWidth, mHeight);
    Image::transitionImageLayout(context, mImage, VK_FORMAT_R8G8B8A8_UNORM,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    _createImageView(context.getDevice());
    _createImageSampler(context.getDevice());
    context.getMemoryManager().freeBuffer(mStagingBuffer);
    mCreated = true;
}

void Image::_createImage(VkDevice device, MemoryManager& manager) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = mWidth;
    imageInfo.extent.height = mHeight;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    if (vkCreateImage(device, &imageInfo, nullptr, &mImage) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create image");
    }

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(device, mImage, &memoryRequirements);

    manager.allocateForImage(mImage, memoryRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

void Image::_createImageView(VkDevice device) {
    VkImageViewCreateInfo imageViewInfo{};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = mImage;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.levelCount = 1;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &imageViewInfo, nullptr, &mImageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image view");
    }
}

void Image::_createImageSampler(VkDevice device) {
    VkSamplerCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.minFilter = VK_FILTER_NEAREST;
    createInfo.magFilter = VK_FILTER_NEAREST;
    createInfo.anisotropyEnable = VK_FALSE;
    createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    if (vkCreateSampler(device, &createInfo, nullptr, &mSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create sampler");
    }
}

void Image::destroy(VulkanContext& context) {
    if (mCreated) {
        vkDestroyImageView(context.getDevice(), mImageView, nullptr);
        vkDestroySampler(context.getDevice(), mSampler, nullptr);
        context.getMemoryManager().freeImage(mImage);
        mCreated = false;
    }
}

VkImage Image::getHandler() const {
    return mImage;
}

VkImageView Image::getViewHandler() const {
    return mImageView;
}

VkSampler Image::getSamplerHandler() const {
    return mSampler;
}
