#include "vulkan/Commands.hpp"

vk::CommandBuffer Commands::beginSingleTime(vk::Device device, CommandPool& commandPool) {
    vk::CommandBufferAllocateInfo allocateInfo;
    allocateInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    allocateInfo.setCommandPool(commandPool.getHandler());
    allocateInfo.setCommandBufferCount(1);

    vk::CommandBuffer commandBuffer = device.allocateCommandBuffers(allocateInfo)[0];

    commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

    return commandBuffer;
}

void Commands::endSingleTime(vk::Device device,
                             CommandPool& commandPool,
                             vk::CommandBuffer commandBuffer,
                             vk::Queue queue) {
    commandBuffer.end();

    queue.submit(vk::SubmitInfo(0, nullptr, nullptr, 1, &commandBuffer), vk::Fence());
    queue.waitIdle();

    device.freeCommandBuffers(commandPool.getHandler(), commandBuffer);
}