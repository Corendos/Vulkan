#ifndef IMAGEVIEW
#define IMAGEVIEW

#include <vulkan/vulkan.h>

class ImageView {
    public:
        ImageView();

        void setFlags(VkImageViewCreateFlags flags);
        void setImage(VkImage image);
        void setImageViewType(VkImageViewType viewType);
        void setFormat(VkFormat format);
        void setComponentsMapping(VkComponentMapping components);
        void setSubresourceRange(VkImageSubresourceRange subresourceRange);

    private:
        VkImageView mImageView;
        VkImageViewCreateInfo mInfo{};
};

#endif