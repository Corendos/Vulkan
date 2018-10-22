#include "vulkan/CommandPool.hpp"

void CommandPool::create(VkDevice device, QueueFamilyIndices indices) {
    if (mCreated) {
        return;
    }

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = indices.graphicsFamily.value();
    poolInfo.flags = 0;

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &mHandler) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool");
    }

    mCreated = true;
}

void CommandPool::destroy(VkDevice device) {
    if (mCreated) {
        vkDestroyCommandPool(device, mHandler, nullptr);
        mCreated = false;
    }
}

VkCommandPool CommandPool::getHandler() const {
    return mHandler;
}