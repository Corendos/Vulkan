#include "resources/TextureManager.hpp"
#include "vulkan/buffer/BufferHelper.hpp"
#include "vulkan/image/ImageHelper.hpp"
#include "vulkan/image/Image.hpp"

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
        mContext->getMemoryManager().freeImage(it->second.getImage().getHandler());
    }
}

Texture& TextureManager::load(std::string name, std::string filename) {
    Image image = _createImage(filename);
    ImageView imageView = _createImageView(image);
    Sampler sampler = _createSampler();

    Texture t;
    t.setImage(image);
    t.setImageView(imageView);
    t.setSampler(sampler);

    mTextures.insert(std::make_pair(name, std::move(t)));

    return mTextures[name];
}

Texture& TextureManager::getTexture(std::string name) {
    return mTextures.at(name);
}

VkBuffer TextureManager::_loadToStaging(std::string& filename,
                                        uint32_t& width,
                                        uint32_t& height) {
    int _width, _height, _bpp;
    uint8_t* pixels = stbi_load(filename.c_str(), &_width, &_height, &_bpp, 4);

    width = static_cast<uint32_t>(_width);
    height = static_cast<uint32_t>(_height);
    
    VkDeviceSize size = width * height * 4;
    VkBuffer stagingBuffer;
    BufferHelper::createBuffer(*mContext, size,
                               VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                               stagingBuffer, filename);

    void* data;
    mContext->getMemoryManager().mapMemory(stagingBuffer, size, &data);
    memcpy(data, pixels, size);
    mContext->getMemoryManager().unmapMemory(stagingBuffer);

    stbi_image_free(pixels);
    return stagingBuffer;
}

Image TextureManager::_createImage(std::string& filename) {
    uint32_t width, height;
    VkBuffer stagingBuffer = _loadToStaging(filename, width, height);

    Image image;
    image.setImageType(VK_IMAGE_TYPE_2D);
    image.setExtent({width, height, 1});
    image.setMipLevels(1);
    image.setArrayLayers(1);
    image.setFormat(VK_FORMAT_R8G8B8A8_UNORM);
    image.setTiling(VK_IMAGE_TILING_OPTIMAL);
    image.setInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED);
    image.setUsage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    image.setSharingMode(VK_SHARING_MODE_EXCLUSIVE);
    image.setSamples(VK_SAMPLE_COUNT_1_BIT);
    image.create(*mContext);

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(mContext->getDevice(), image.getHandler(), &memoryRequirements);

    mContext->getMemoryManager().allocateForImage(image.getHandler(), memoryRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, filename);

    ImageHelper::transitionImageLayout(*mContext, image.getHandler(), VK_FORMAT_R8G8B8A8_UNORM,
                                 VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    BufferHelper::copyBufferToImage(*mContext, mContext->getTransferCommandPool(),
                                    mContext->getTransferQueue(), stagingBuffer,
                                    image.getHandler(), width, height);
    ImageHelper::transitionImageLayout(*mContext, image.getHandler(), VK_FORMAT_R8G8B8A8_UNORM,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    mContext->getMemoryManager().freeBuffer(stagingBuffer);
    return image;
}

ImageView TextureManager::_createImageView(Image& image) {
    ImageView imageView;
    imageView.setImage(image.getHandler());
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
    return imageView;
}

Sampler TextureManager::_createSampler() {
    Sampler sampler;
    sampler.setMinFilter(VK_FILTER_NEAREST);
    sampler.setMagFilter(VK_FILTER_NEAREST);
    sampler.setAnisotropyEnable(VK_FALSE);
    sampler.setAddressModeU(VK_SAMPLER_ADDRESS_MODE_REPEAT);
    sampler.setAddressModeV(VK_SAMPLER_ADDRESS_MODE_REPEAT);
    sampler.setAddressModeW(VK_SAMPLER_ADDRESS_MODE_REPEAT);
    sampler.create(mContext->getDevice());
    return sampler;
}
