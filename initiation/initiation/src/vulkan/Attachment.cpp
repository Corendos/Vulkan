#include "vulkan/Attachment.hpp"

VkAttachmentDescription Attachment::getDescription() const {
    return mDescription;
}

void Attachment::setFormat(VkFormat format) {
    mDescription.format = format;
}

void Attachment::setSamples(VkSampleCountFlagBits samples) {
    mDescription.samples = samples;
}

void Attachment::setLoadOp(VkAttachmentLoadOp op) {
    mDescription.loadOp = op;
}

void Attachment::setStoreOp(VkAttachmentStoreOp op) {
    mDescription.storeOp = op;
}

void Attachment::setStencilLoadOp(VkAttachmentLoadOp op) {
    mDescription.stencilLoadOp = op;
}

void Attachment::setStencilStoreOp(VkAttachmentStoreOp op) {
    mDescription.stencilStoreOp = op;
}

void Attachment::setInitialLayout(VkImageLayout layout) {
    mDescription.initialLayout = layout;
}

void Attachment::setFinalLayout(VkImageLayout layout) {
    mDescription.finalLayout = layout;
}