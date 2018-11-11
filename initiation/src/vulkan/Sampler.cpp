#include "vulkan/Sampler.hpp"

Sampler::Sampler() {
    mInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
}

void Sampler::create(VkDevice device) {
    if (vkCreateSampler(device, &mInfo, nullptr, &mSampler) != VK_SUCCESS) {
        throw std::runtime_error("faile dto create sampler");
    }
}

void Sampler::destroy(VkDevice device) {
    vkDestroySampler(device, mSampler, nullptr);
}

VkSampler Sampler::getHandler() const {
    return mSampler;
}

void Sampler::setFlags(VkSamplerCreateFlags flags) {
    mInfo.flags = flags;
}

void Sampler::setMagFilter(VkFilter magFilter) {
    mInfo.magFilter = magFilter;
}

void Sampler::setMinFilter(VkFilter minFilter) {
    mInfo.minFilter = minFilter;
}

void Sampler::setMipmapMode(VkSamplerMipmapMode mipmapMode) {
    mInfo.mipmapMode = mipmapMode;
}

void Sampler::setAddressModeU(VkSamplerAddressMode addressModeU) {
    mInfo.addressModeU = addressModeU;
}

void Sampler::setAddressModeV(VkSamplerAddressMode addressModeV) {
    mInfo.addressModeV = addressModeV;
}

void Sampler::setAddressModeW(VkSamplerAddressMode addressModeW) {
    mInfo.addressModeW = addressModeW;
}

void Sampler::setMipLodBias(float mipLodBias) {
    mInfo.mipLodBias = mipLodBias;
}

void Sampler::setAnisotropyEnable(VkBool32 anisotropyEnable) {
    mInfo.anisotropyEnable = anisotropyEnable;
}

void Sampler::setMaxAnisotropy(float maxAnisotropy) {
    mInfo.maxAnisotropy = maxAnisotropy;
}

void Sampler::setCompareEnable(VkBool32 compareEnable) {
    mInfo.compareEnable = compareEnable;
}

void Sampler::setCompareOp(VkCompareOp compareOp) {
    mInfo.compareOp = compareOp;
}

void Sampler::setMinLod(float minLod) {
    mInfo.minLod = minLod;
}

void Sampler::setMaxLod(float maxLod) {
    mInfo.maxLod = maxLod;
}

void Sampler::setBorderColor(VkBorderColor borderColor) {
    mInfo.borderColor = borderColor;
}

void Sampler::setUnnormalizedCoordinates(VkBool32 unnormalizedCoordinates) {
    mInfo.unnormalizedCoordinates = unnormalizedCoordinates;
}
