#ifndef IMAGEVIEW
#define IMAGEVIEW

#include <stdexcept>
#include <vulkan/vulkan.h>

class ImageView {
    public:
        ImageView();

        void create(VkDevice device);
        void destroy(VkDevice device);

        VkImageView getHandler() const;

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