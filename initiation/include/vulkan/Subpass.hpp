#ifndef SUBPASS
#define SUBPASS

#include <vector>

#include <vulkan/vulkan.hpp>

#include "vulkan/ColorAttachment.hpp"
#include "vulkan/DepthAttachment.hpp"

class Subpass {
    public:
        void setFlags(VkSubpassDescriptionFlags flags);
        void setBindPoint(VkPipelineBindPoint bindPoint);
        void setInputAttachments(std::vector<VkAttachmentReference>& inputAttachments);
        void setColorAttachments(std::vector<VkAttachmentReference>& colorAttachments);
        void setResolveAttachments(std::vector<VkAttachmentReference>& resolveAttachments);
        void setDepthAttachment(VkAttachmentReference depthStencilAttachment);

        VkSubpassDescription getDescription() const;

    private:
        VkSubpassDescription mDescription{};
        std::vector<VkAttachmentReference> mInputAttachments;
        std::vector<VkAttachmentReference> mColorAttachments;
        std::vector<VkAttachmentReference> mResolveAttachments;
        VkAttachmentReference mDepthStencilAttachment;
};

#endif