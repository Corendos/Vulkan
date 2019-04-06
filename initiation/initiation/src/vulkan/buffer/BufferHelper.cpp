#include "vulkan/buffer/BufferHelper.hpp"

#include "vulkan/Commands.hpp"

void BufferHelper::createBuffer(VulkanContext& context,
                                vk::DeviceSize size,
                                vk::BufferUsageFlags usage,
                                vk::SharingMode sharingMode,
                                vk::MemoryPropertyFlags properties,
                                vk::Buffer& buffer,
                                std::string name) {
    vk::BufferCreateInfo bufferInfo;
    bufferInfo.setSize(size);
    bufferInfo.setUsage(usage);
    bufferInfo.setSharingMode(sharingMode);
    buffer = context.getDevice().createBuffer(bufferInfo);

    vk::MemoryRequirements memoryRequirements = context.getDevice().getBufferMemoryRequirements(buffer);

    context.getMemoryManager().allocateForBuffer(buffer, memoryRequirements, properties, name);
}

void BufferHelper::copyBuffer(VulkanContext& context,
                              CommandPool& commandPool,
                              vk::Queue queue,
                              vk::Buffer srcBuffer,
                              vk::Buffer dstBuffer,
                              vk::DeviceSize size) {
    vk::CommandBufferAllocateInfo allocateInfo;
    allocateInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    allocateInfo.setCommandPool(commandPool.getHandler());
    allocateInfo.setCommandBufferCount(1);

    vk::CommandBuffer commandBuffer = context.getDevice().allocateCommandBuffers(allocateInfo)[0];

    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    commandBuffer.begin(beginInfo);

    vk::BufferCopy copyRegion;
    copyRegion.setSize(size);
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);
    commandBuffer.end();

    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBufferCount(1);
    submitInfo.setPCommandBuffers(&commandBuffer);

    queue.submit(submitInfo, vk::Fence());
    queue.waitIdle();

    context.getDevice().freeCommandBuffers(commandPool.getHandler(), commandBuffer);
}

void BufferHelper::copyBufferToImage(VulkanContext& context,
                                     CommandPool& commandPool,
                                     vk::Queue queue,
                                     vk::Buffer buffer,
                                     vk::Image image,
                                     uint32_t width,
                                     uint32_t height) {
    vk::CommandBufferAllocateInfo allocateInfo;
    allocateInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    allocateInfo.setCommandPool(commandPool.getHandler());
    allocateInfo.setCommandBufferCount(1);

    vk::CommandBuffer commandBuffer = context.getDevice().allocateCommandBuffers(allocateInfo)[0];

    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    commandBuffer.begin(beginInfo);

    vk::BufferImageCopy region; 
    region.setImageSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1));
    region.setImageOffset({0, 0, 0});
    region.setImageExtent({width, height, 1});

    commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
    commandBuffer.end();

    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBufferCount(1);
    submitInfo.setPCommandBuffers(&commandBuffer);

    queue.submit(submitInfo, vk::Fence());
    queue.waitIdle();

    context.getDevice().freeCommandBuffers(commandPool.getHandler(), commandBuffer);
}