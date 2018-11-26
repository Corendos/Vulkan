#include "vulkan/BufferHelper.hpp"

#include "vulkan/Commands.hpp"

void BufferHelper::createBuffer(VulkanContext& context,
                                VkDeviceSize size,
                                VkBufferUsageFlags usage,
                                VkMemoryPropertyFlags properties,
                                VkBuffer& buffer,
                                std::string name) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(context.getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create vertex buffer");
    }

    VkMemoryRequirements memoryRequirements{};
    vkGetBufferMemoryRequirements(context.getDevice(), buffer, &memoryRequirements);

    context.getMemoryManager().allocateForBuffer(buffer, memoryRequirements, properties, name);
}

void BufferHelper::copyBuffer(VulkanContext& context,
                              CommandPool& commandPool,
                              VkQueue queue,
                              VkBuffer srcBuffer,
                              VkBuffer dstBuffer,
                              VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandPool = commandPool.getHandler();
    allocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer{};
    vkAllocateCommandBuffers(context.getDevice(), &allocateInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(context.getDevice(), commandPool.getHandler(), 1, &commandBuffer);
}

void BufferHelper::copyBufferToImage(VulkanContext& context,
                                     CommandPool& commandPool,
                                     VkQueue queue,
                                     VkBuffer buffer,
                                     VkImage image,
                                     uint32_t width,
                                     uint32_t height) {
    VkCommandBuffer commandBuffer = Commands::beginSingleTime(context.getDevice(), commandPool);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(
        commandBuffer, buffer, image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &region);
    Commands::endSingleTime(context.getDevice(), commandPool, commandBuffer, queue);
}