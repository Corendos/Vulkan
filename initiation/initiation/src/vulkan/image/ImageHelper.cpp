#include "vulkan/image/ImageHelper.hpp"

#include "vulkan/Commands.hpp"

vk::Image ImageHelper::create(VulkanContext& context,
                   uint32_t width, uint32_t height,
                   vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                   vk::MemoryPropertyFlags properties) {
    vk::ImageCreateInfo imageCreateInfo;
    imageCreateInfo.setImageType(vk::ImageType::e2D);
    imageCreateInfo.setExtent({width, height, 1});
    imageCreateInfo.setMipLevels(1);
    imageCreateInfo.setArrayLayers(1);
    imageCreateInfo.setFormat(format);
    imageCreateInfo.setTiling(tiling);
    imageCreateInfo.setInitialLayout(vk::ImageLayout::eUndefined);
    imageCreateInfo.setUsage(usage);
    imageCreateInfo.setSharingMode(vk::SharingMode::eExclusive);
    imageCreateInfo.setSamples(vk::SampleCountFlagBits::e1);

    vk::Image image = context.getDevice().createImage(imageCreateInfo);

    vk::MemoryRequirements memoryRequirements = context.getDevice().getImageMemoryRequirements(image);

    context.getMemoryManager().allocateForImage(
        image, memoryRequirements, vk::MemoryPropertyFlagBits::eDeviceLocal, "Image");
    
    return image;
}

void ImageHelper::transitionImageLayout(VulkanContext& context,
                                  vk::Image image,
                                  vk::Format format,
                                  vk::ImageLayout oldLayout,
                                  vk::ImageLayout newLayout) {
    vk::CommandPool& transferCommandPool = context.getTransferCommandPool();

    vk::CommandBuffer commandBuffer = Commands::beginSingleTime(context.getDevice(), transferCommandPool);

    vk::ImageMemoryBarrier memoryBarrier;
    memoryBarrier.setOldLayout(oldLayout);
    memoryBarrier.setNewLayout(newLayout);
    memoryBarrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
    memoryBarrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
    memoryBarrier.setImage(image);
    memoryBarrier.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
        memoryBarrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        memoryBarrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
        memoryBarrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal) {
        memoryBarrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
    } else {
        throw std::runtime_error("Unsupported layout transition");
    }

    if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        memoryBarrier.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eDepth);

        if (hasStencilComponent(format)) {
            memoryBarrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
        }
    } else {
        memoryBarrier.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
    }

    commandBuffer.pipelineBarrier(
        sourceStage, destinationStage,
        vk::DependencyFlags(), {}, {}, memoryBarrier);

    Commands::endSingleTime(context.getDevice(),
                            transferCommandPool,
                            commandBuffer,
                            context.getGraphicsQueue());
}

bool ImageHelper::hasStencilComponent(vk::Format format) {
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}