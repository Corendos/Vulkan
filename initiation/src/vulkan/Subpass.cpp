#include "vulkan/Subpass.hpp"

void Subpass::setBindPoint(VkPipelineBindPoint bindPoint) {
    mDescription.pipelineBindPoint = bindPoint;
}

void Subpass::addAttachment(ColorAttachment colorAttachment, DepthAttachment depthAttachment) {
    mColorAttachmentReferences.push_back(colorAttachment.getReference());
    mDepthAttachmentReferences.push_back(depthAttachment.getReference());

    mDescription.colorAttachmentCount += 1;
    mDescription.pColorAttachments = mColorAttachmentReferences.data();
    mDescription.pDepthStencilAttachment = mDepthAttachmentReferences.data();
}

VkSubpassDescription Subpass::getDescription() const {
    return mDescription;
}