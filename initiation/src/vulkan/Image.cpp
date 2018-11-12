#include "vulkan/Image.hpp"
#include "vulkan/ImageHelper.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Image::Image() {
    mInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
}

void Image::create(VulkanContext& context) {
    if (vkCreateImage(context.getDevice(), &mInfo, nullptr, &mImage) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create image");
    }
    mCreated = true;
}

void Image::destroy(VulkanContext& context) {
    if (mCreated) {
        context.getMemoryManager().freeImage(mImage);
        mCreated = false;
    }
}

void Image::setImageType(VkImageType imageType) {
    mInfo.imageType = imageType;
}

void Image::setExtent(VkExtent3D extent) {
    mInfo.extent = extent;
}

void Image::setMipLevels(uint32_t mipLevels) {
    mInfo.mipLevels = mipLevels;
}

void Image::setArrayLayers(uint32_t arrayLayers) {
    mInfo.arrayLayers = arrayLayers;
}

void Image::setFormat(VkFormat format) {
    mInfo.format = format;
}

void Image::setTiling(VkImageTiling tiling) {
    mInfo.tiling = tiling;
}

void Image::setInitialLayout(VkImageLayout initialLayout) {
    mInfo.initialLayout = initialLayout;
}

void Image::setUsage(VkImageUsageFlags usage) {
    mInfo.usage = usage;
}

void Image::setSharingMode(VkSharingMode sharingMode) {
    mInfo.sharingMode = sharingMode;
}

void Image::setSamples(VkSampleCountFlagBits samples) {
    mInfo.samples = samples;
}

void Image::setFlags(VkFlags flags) {
    mInfo.flags = flags;
}


VkImage Image::getHandler() {
    return mImage;
}
