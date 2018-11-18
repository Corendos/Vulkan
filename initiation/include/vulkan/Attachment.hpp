#ifndef ATTACHMENT
#define ATTACHMENT

#include <vulkan/vulkan.h>

class Attachment {
    public:
        VkAttachmentDescription getDescription() const;

        void setFormat(VkFormat format);
        void setSamples(VkSampleCountFlagBits samples);
        void setInitialLayout(VkImageLayout layout);
        void setFinalLayout(VkImageLayout layout);
        void setLoadOp(VkAttachmentLoadOp op);
        void setStoreOp(VkAttachmentStoreOp op);
        void setStencilLoadOp(VkAttachmentLoadOp op);
        void setStencilStoreOp(VkAttachmentStoreOp op);
    
    protected:
        VkAttachmentDescription mDescription{};
};

#endif