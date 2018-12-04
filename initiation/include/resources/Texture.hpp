#ifndef TEXTURE
#define TEXTURE

#include <vulkan/vulkan.h>

#include "vulkan/image/Image.hpp"
#include "vulkan/image/ImageView.hpp"
#include "vulkan/Sampler.hpp"

class Texture {
    public:
        Texture() = default;
        Texture(VkImage image, ImageView imageView, Sampler sampler);
        Image& getImage();
        ImageView& getImageView();
        Sampler& getSampler();

        void setImageView(ImageView imageView);
        void setSampler(Sampler sampler);
        void setImage(Image image);
        
    private:
        Image mImage;
        ImageView mImageView;
        Sampler mSampler;
};

#endif