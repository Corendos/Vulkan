#include "vulkan/RenderPass.hpp"

void RenderPass::create(VkDevice device) {
    if (mCreated) {
        return;
    }

    mInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    mInfo.attachmentCount = static_cast<uint32_t>(mAttachmentsDescription.size());
    mInfo.pAttachments = mAttachmentsDescription.data();
    mInfo.subpassCount = 1;
    mInfo.pSubpasses = mSubpassesDescription.data();
    mInfo.dependencyCount = 1;
    mInfo.pDependencies = mSubpassesDependency.data();

    if (vkCreateRenderPass(device, &mInfo, nullptr, &mHandler) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass");
    }

    mCreated = true;
}

void RenderPass::destroy(VkDevice device) {
    if (mCreated) {
        vkDestroyRenderPass(device, mHandler, nullptr);
        mAttachmentsDescription.clear();
        mAttachmentsReference.clear();
        mSubpassesDependency.clear();
        mSubpassesDescription.clear();
        mCreated = false;
    }
}

void RenderPass::setAttachments(std::vector<Attachment> attachments) {
    mAttachmentsDescription.resize(attachments.size());
    mAttachmentsReference.resize(attachments.size());

    std::transform(
        attachments.begin(), attachments.end(),
        mAttachmentsDescription.begin(), [](const Attachment& a) { return a.getDescription(); });
    std::transform(
        attachments.begin(), attachments.end(),
        mAttachmentsReference.begin(), [](const Attachment& a) { return a.getReference(); });
}

void RenderPass::addSubpass(VkSubpassDescription description) {
    mSubpassesDescription.push_back(description);
}

void RenderPass::addSubpassDependency(VkSubpassDependency dependency) {
    mSubpassesDependency.push_back(dependency);
}

VkRenderPass RenderPass::getHandler() const {
    return mHandler;
}