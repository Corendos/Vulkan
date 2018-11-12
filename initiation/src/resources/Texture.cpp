#include "resources/Texture.hpp"


Image& Texture::getImage() {
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

void Texture::setImage(Image image) {
    mImage = image;
}
