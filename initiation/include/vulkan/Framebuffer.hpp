#ifndef FRAMEBUFFER
#define FRAMEBUFFER

#include <vector>
#include <stdexcept>

#include "vulkan/vulkan.h"

class Framebuffer {
    public:
        Framebuffer();

        void create(VkDevice device);
        void destroy(VkDevice device);

        void setFlags(VkFramebufferCreateFlags flags);
        void setRenderPass(VkRenderPass renderPass);
        void setAttachments(std::vector<VkImageView>& attachments);
        void setWidth(uint32_t width);
        void setHeight(uint32_t height);
        void setLayers(uint32_t layers);

        VkFramebuffer getHandler();

    private:
        VkFramebuffer mHandler;
        VkFramebufferCreateInfo mInfo{};

        std::vector<VkImageView> mAttachments;
};

#endif