#include "resources/TextureManager.hpp"
#include "vulkan/BufferHelper.hpp"
#include "vulkan/Image.hpp"

#include <cstring>

#ifndef STB_LOADED
#define STB_LOADED
#include "stb_image.h"
#endif

void TextureManager::create(VulkanContext& context) {
    mContext = &context;
}

void TextureManager::destroy() {
    for (auto it{mTextures.begin()};it != mTextures.end();++it) {
        it->second.getImageView().destroy(mContext->getDevice());
        it->second.getSampler().destroy(mContext->getDevice());
        vkDestroyImage(mContext->getDevice(), it->second.getImage(), nullptr);
    }
}

Texture& TextureManager::load(std::string name, std::string filename) {
    Texture t;

    int width, height, bpp;
    uint8_t* pixels = stbi_load(filename.c_str(), &width, &height, &bpp, 4);

    uint32_t uWidth = static_cast<uint32_t>(width);
    uint32_t uHeight = static_cast<uint32_t>(height);
    uint32_t uBpp = static_cast<uint32_t>(bpp);
    
    VkDeviceSize size = uWidth * uHeight * 4;
    VkBuffer stagingBuffer;
    BufferHelper::createBuffer(*mContext, size,
                               VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                               stagingBuffer);

    void* data;
    mContext->getMemoryManager().mapMemory(stagingBuffer, size, &data);
    memcpy(data, pixels, size);
    mContext->getMemoryManager().unmapMemory(stagingBuffer);

    stbi_image_free(pixels);

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = uWidth;
    imageInfo.extent.height = uHeight;
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

    if (vkCreateImage(mContext->getDevice(), &imageInfo, nullptr, t.getImagePtr()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create image");
    }

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(mContext->getDevice(), t.getImage(), &memoryRequirements);

    mContext->getMemoryManager().allocateForImage(t.getImageRef(), memoryRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    Image::transitionImageLayout(*mContext, t.getImage(), VK_FORMAT_R8G8B8A8_UNORM,
                                 VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    BufferHelper::copyBufferToImage(*mContext, mContext->getTransferCommandPool(),
                                    mContext->getTransferQueue(), stagingBuffer,
                                    t.getImage(), uWidth, uHeight);
    Image::transitionImageLayout(*mContext, t.getImage(), VK_FORMAT_R8G8B8A8_UNORM,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    mContext->getMemoryManager().freeBuffer(stagingBuffer);

    ImageView imageView;
    imageView.setImage(t.getImage());
    imageView.setImageViewType(VK_IMAGE_VIEW_TYPE_2D);
    imageView.setFormat(VK_FORMAT_R8G8B8A8_UNORM);
    VkImageSubresourceRange subresourceRange;
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 1;
    imageView.setSubresourceRange(subresourceRange);
    imageView.create(mContext->getDevice());

    t.setImageView(imageView);

    Sampler sampler;
    sampler.setMinFilter(VK_FILTER_NEAREST);
    sampler.setMagFilter(VK_FILTER_NEAREST);
    sampler.setAnisotropyEnable(VK_FALSE);
    sampler.setAddressModeU(VK_SAMPLER_ADDRESS_MODE_REPEAT);
    sampler.setAddressModeV(VK_SAMPLER_ADDRESS_MODE_REPEAT);
    sampler.setAddressModeW(VK_SAMPLER_ADDRESS_MODE_REPEAT);
    sampler.create(mContext->getDevice());

    t.setSampler(sampler);

    mTextures.insert(std::make_pair(name, std::move(t)));

    return mTextures[name];
}

Texture& TextureManager::getTexture(std::string name) {
    return mTextures.at(name);
}