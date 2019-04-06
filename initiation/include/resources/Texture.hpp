#ifndef TEXTURE
#define TEXTURE

#include <vulkan/vulkan.hpp>

class Texture {
    public:
        Texture() = default;
        Texture(vk::Image image, vk::ImageView imageView, vk::Sampler sampler);
        vk::Image& getImage();
        vk::ImageView& getImageView();
        vk::Sampler& getSampler();

        void setImageView(vk::ImageView imageView);
        void setSampler(vk::Sampler sampler);
        void setImage(vk::Image image);
        
    private:
        vk::Image mImage;
        vk::ImageView mImageView;
        vk::Sampler mSampler;
};

#endif