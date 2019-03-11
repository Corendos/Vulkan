#include "vulkan/CommandPool.hpp"

void CommandPool::create(VkDevice device, uint32_t queueFamilyIndex) {
    if (mCreated) {
        return;
    }
    // TODO: give the possibility to specify the queue family index
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

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

void CommandPool::lock() {
    mMutex.lock();
}

void CommandPool::unlock() {
    mMutex.unlock();
}