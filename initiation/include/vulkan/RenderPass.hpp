#ifndef RENDERPASS
#define RENDERPASS

#include <vector>
#include <stdexcept>
#include <algorithm>

#include <vulkan/vulkan.h>

#include "vulkan/Attachment.hpp"

class RenderPass {
    public:
        void create(VkDevice device);
        void destroy(VkDevice device);

        void setAttachments(std::vector<Attachment>& attachments);
        void addSubpass(VkSubpassDescription description);
        void addSubpassDependency(VkSubpassDependency dependency);

        VkRenderPass getHandler() const;

    private:
        VkRenderPass mHandler;
        VkRenderPassCreateInfo mInfo{};
        std::vector<VkAttachmentDescription> mAttachmentsDescription;
        std::vector<VkSubpassDescription> mSubpassesDescription;
        std::vector<VkSubpassDependency> mSubpassesDependency;

        bool mCreated{false};
};

#endif