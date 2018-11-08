#include "vulkan/DescriptorPool.hpp"

void DescriptorPool::create(VkDevice device) {
    if (vkCreateDescriptorPool(device, &mInfo, nullptr, &mHandler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool");
    }
}

void DescriptorPool::destroy(VkDevice device) {
    vkDestroyDescriptorPool(device, mHandler, nullptr);
}

void DescriptorPool::setFlags(VkDescriptorPoolCreateFlagBits flags) {
    mInfo.flags = flags;
}

void DescriptorPool::setMaxSets(uint32_t count) {
    mInfo.maxSets = count;
}

void DescriptorPool::setPoolSizes(std::vector<VkDescriptorPoolSize> poolSizes) {
    mPoolSizes = poolSizes;
    mInfo.poolSizeCount = static_cast<uint32_t>(mPoolSizes.size());
    mInfo.pPoolSizes = mPoolSizes.data();
}

VkDescriptorPool DescriptorPool::getHandler() const {
    return mHandler;
}