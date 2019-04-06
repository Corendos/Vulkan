#include "resources/Texture.hpp"


vk::Image& Texture::getImage() {
    return mImage;
}

vk::ImageView& Texture::getImageView() {
    return mImageView;
}

vk::Sampler& Texture::getSampler() {
    return mSampler;
}

void Texture::setImageView(vk::ImageView imageView) {
    mImageView = imageView;
}

void Texture::setSampler(vk::Sampler sampler) {
    mSampler = sampler;
}

void Texture::setImage(vk::Image image) {
    mImage = image;
}
