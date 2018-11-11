#include "resources/Texture.hpp"

VkImage* Texture::getImagePtr() {
    return &mImage;
}

VkImage& Texture::getImageRef() {
    return mImage;
}

VkImage Texture::getImage() const {
    return mImage;
}

ImageView& Texture::getImageView() {
    return mImageView;
}

Sampler& Texture::getSampler() {
    return mSampler;
}

void Texture::setImageView(ImageView imageView) {
    mImageView = imageView;
}

void Texture::setSampler(Sampler sampler) {
    mSampler = sampler;
}
