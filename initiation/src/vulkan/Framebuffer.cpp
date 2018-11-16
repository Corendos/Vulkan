#include "vulkan/Framebuffer.hpp"

Framebuffer::Framebuffer() {
    mInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
}

void Framebuffer::create(VkDevice device) {
    if (vkCreateFramebuffer(device, &mInfo, nullptr, &mHandler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer");
    }
}

void Framebuffer::destroy(VkDevice device) {
    vkDestroyFramebuffer(device, mHandler, nullptr);
}

void Framebuffer::setFlags(VkFramebufferCreateFlags flags) {
    mInfo.flags = flags;
}

void Framebuffer::setRenderPass(VkRenderPass renderPass) {
    mInfo.renderPass = renderPass;
}

void Framebuffer::setAttachments(std::vector<VkImageView>& attachments) {
    mAttachments = attachments;
    mInfo.attachmentCount = static_cast<uint32_t>(mAttachments.size());
    mInfo.pAttachments = mAttachments.data();
}

void Framebuffer::setWidth(uint32_t width) {
    mInfo.width = width;
}

void Framebuffer::setHeight(uint32_t height) {
    mInfo.height = height;
}

void Framebuffer::setLayers(uint32_t layers) {
    mInfo.layers = layers;
}

VkFramebuffer Framebuffer::getHandler() {
    return mHandler;
}