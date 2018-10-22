#ifndef RENDERPASS
#define RENDERPASS

#include <vector>
#include <stdexcept>

#include <vulkan/vulkan.h>

class RenderPass {
    public:
        void create(VkDevice device);
        void destroy(VkDevice device);

        void addAttachment(VkAttachmentDescription description, VkAttachmentReference attachmentReference);
        void addSubpass(VkSubpassDescription description);
        void addSubpassDependency(VkSubpassDependency dependency);

        VkRenderPass getHandler() const;

    private:
        VkRenderPass mHandler;
        VkRenderPassCreateInfo mInfo{};
        std::vector<VkAttachmentDescription> mAttachmentsDescription;
        std::vector<VkAttachmentReference> mAttachmentsReference;
        std::vector<VkSubpassDescription> mSubpassesDescription;
        std::vector<VkSubpassDependency> mSubpassesDependency;

        bool mCreated{false};
};

#endif