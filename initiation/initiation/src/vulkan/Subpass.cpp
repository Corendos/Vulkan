#include "vulkan/Subpass.hpp"

void Subpass::setFlags(VkSubpassDescriptionFlags flags) {
    mDescription.flags = flags;
}

void Subpass::setBindPoint(VkPipelineBindPoint bindPoint) {
    mDescription.pipelineBindPoint = bindPoint;
}

void Subpass::setInputAttachments(std::vector<VkAttachmentReference>& inputAttachments) {
    mInputAttachments = inputAttachments;
    mDescription.inputAttachmentCount = static_cast<uint32_t>(mInputAttachments.size());
    mDescription.pInputAttachments = mInputAttachments.data();
}

void Subpass::setColorAttachments(std::vector<VkAttachmentReference>& colorAttachments) {
    mColorAttachments = colorAttachments;
    mDescription.colorAttachmentCount = static_cast<uint32_t>(mColorAttachments.size());
    mDescription.pColorAttachments = mColorAttachments.data();
}

void Subpass::setResolveAttachments(std::vector<VkAttachmentReference>& resolveAttachments) {
    assert(resolveAttachments.size() == mColorAttachments.size());
    mResolveAttachments = resolveAttachments;
    mDescription.pResolveAttachments = mResolveAttachments.data();
}

void Subpass::setDepthAttachment(VkAttachmentReference depthStencilAttachment) {
    mDepthStencilAttachment = depthStencilAttachment;
    mDescription.pDepthStencilAttachment = &mDepthStencilAttachment;
}

VkSubpassDescription Subpass::getDescription() const {
    return mDescription;
}