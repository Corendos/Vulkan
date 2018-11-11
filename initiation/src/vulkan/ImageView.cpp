#include "vulkan/ImageView.hpp"

ImageView::ImageView() {
    mInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
}


void ImageView::create(VkDevice device) {
    if (vkCreateImageView(device, &mInfo, nullptr, &mImageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image view");
    }
}

void ImageView::destroy(VkDevice device) {
    vkDestroyImageView(device, mImageView, nullptr);
}

VkImageView ImageView::getHandler() const {
    return mImageView;
}

void ImageView::setFlags(VkImageViewCreateFlags flags) {
    mInfo.flags = flags;
}

void ImageView::setImage(VkImage image) {
    mInfo.image = image;
}

void ImageView::setImageViewType(VkImageViewType viewType) {
    mInfo.viewType = viewType;
}

void ImageView::setFormat(VkFormat format) {
    mInfo.format = format;
}

void ImageView::setComponentsMapping(VkComponentMapping components) {
    mInfo.components = components;
}

void ImageView::setSubresourceRange(VkImageSubresourceRange subresourceRange) {
    mInfo.subresourceRange = subresourceRange;
}
