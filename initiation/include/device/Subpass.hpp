#ifndef SUBPASS
#define SUBPASS

#include <vector>

#include <vulkan/vulkan.hpp>

#include "device/ColorAttachment.hpp"
#include "device/DepthAttachment.hpp"

class Subpass {
    public:
        void setBindPoint(VkPipelineBindPoint bindPoint);
        void addAttachment(ColorAttachment colorAttachment, DepthAttachment depthAttachment);

        VkSubpassDescription getDescription() const;

    private:
        VkSubpassDescription mDescription{};
        std::vector<VkAttachmentReference> mColorAttachmentReferences;
        std::vector<VkAttachmentReference> mDepthAttachmentReferences;
};

#endif