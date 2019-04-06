#include "resources/TextureManager.hpp"
#include "vulkan/buffer/BufferHelper.hpp"
#include "vulkan/image/ImageHelper.hpp"

#include <cstring>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void TextureManager::create(VulkanContext& context) {
    mContext = &context;
}

void TextureManager::destroy() {
    for (auto it{mTextures.begin()};it != mTextures.end();++it) {
        mContext->getDevice().destroyImageView(it->second.getImageView());
        mContext->getDevice().destroySampler(it->second.getSampler());
        mContext->getMemoryManager().freeImage(it->second.getImage());
    }
}

Texture& TextureManager::load(std::string name, std::string filename) {
    vk::Image image = _createImage(filename);
    vk::ImageView imageView = _createImageView(image);
    vk::Sampler sampler = _createSampler();

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

vk::Buffer TextureManager::_loadToStaging(std::string& filename,
                                        uint32_t& width,
                                        uint32_t& height) {
    int _width, _height, _bpp;
    uint8_t* pixels = stbi_load(filename.c_str(), &_width, &_height, &_bpp, 4);

    width = static_cast<uint32_t>(_width);
    height = static_cast<uint32_t>(_height);
    
    vk::DeviceSize size = width * height * 4;
    vk::Buffer stagingBuffer;
    BufferHelper::createBuffer(*mContext, size,
                               vk::BufferUsageFlagBits::eTransferSrc,
                               vk::SharingMode::eExclusive,
                               vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                               stagingBuffer, filename);

    void* data;
    mContext->getMemoryManager().mapMemory(stagingBuffer, size, &data);
    memcpy(data, pixels, size);
    mContext->getMemoryManager().unmapMemory(stagingBuffer);

    stbi_image_free(pixels);
    return stagingBuffer;
}

vk::Image TextureManager::_createImage(std::string& filename) {
    uint32_t width, height;
    vk::Buffer stagingBuffer = _loadToStaging(filename, width, height);

    vk::ImageCreateInfo imageCreateInfo;
    imageCreateInfo.setImageType(vk::ImageType::e2D);
    imageCreateInfo.setExtent({width, height, 1});
    imageCreateInfo.setMipLevels(1);
    imageCreateInfo.setArrayLayers(1);
    imageCreateInfo.setFormat(vk::Format::eR8G8B8A8Unorm);
    imageCreateInfo.setTiling(vk::ImageTiling::eOptimal);
    imageCreateInfo.setInitialLayout(vk::ImageLayout::eUndefined);
    imageCreateInfo.setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);
    imageCreateInfo.setSharingMode(vk::SharingMode::eExclusive);
    imageCreateInfo.setSamples(vk::SampleCountFlagBits::e1);

    vk::Image image = mContext->getDevice().createImage(imageCreateInfo);

    vk::MemoryRequirements memoryRequirements = mContext->getDevice().getImageMemoryRequirements(image);

    mContext->getMemoryManager().allocateForImage(
        image, memoryRequirements, vk::MemoryPropertyFlagBits::eDeviceLocal, filename);

    ImageHelper::transitionImageLayout(*mContext, image, vk::Format::eR8G8B8A8Unorm,
                                 vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    vk::CommandPool& transferCommandPool = mContext->getTransferCommandPool();

    BufferHelper::copyBufferToImage(*mContext, transferCommandPool,
                                    mContext->getTransferQueue(), stagingBuffer,
                                    image, width, height);

    ImageHelper::transitionImageLayout(*mContext, image, vk::Format::eR8G8B8A8Unorm,
                                 vk::ImageLayout::eTransferDstOptimal,
                                 vk::ImageLayout::eShaderReadOnlyOptimal);
    mContext->getMemoryManager().freeBuffer(stagingBuffer);
    return image;
}

vk::ImageView TextureManager::_createImageView(vk::Image& image) {
    vk::ImageViewCreateInfo createInfo;
    createInfo.setImage(image);
    createInfo.setViewType(vk::ImageViewType::e2D);
    createInfo.setFormat(vk::Format::eR8G8B8A8Unorm);
    createInfo.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
    return mContext->getDevice().createImageView(createInfo);
}

vk::Sampler TextureManager::_createSampler() {
    vk::SamplerCreateInfo createInfo;
    createInfo.setMinFilter(vk::Filter::eNearest);
    createInfo.setMagFilter(vk::Filter::eNearest);
    createInfo.setAnisotropyEnable(VK_FALSE);
    createInfo.setAddressModeU(vk::SamplerAddressMode::eRepeat);
    createInfo.setAddressModeV(vk::SamplerAddressMode::eRepeat);
    createInfo.setAddressModeW(vk::SamplerAddressMode::eRepeat);
    return mContext->getDevice().createSampler(createInfo);
}
