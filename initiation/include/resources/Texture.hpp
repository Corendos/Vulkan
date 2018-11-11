#ifndef TEXTURE
#define TEXTURE

#include <vulkan/vulkan.h>

#include "vulkan/ImageView.hpp"
#include "vulkan/Sampler.hpp"

class Texture {
    public:
        Texture() = default;
        Texture(VkImage image, ImageView imageView, Sampler sampler);
        VkImage* getImagePtr();
        VkImage& getImageRef();
        VkImage getImage() const;
        ImageView& getImageView();
        Sampler& getSampler();

        void setImageView(ImageView imageView);
        void setSampler(Sampler sampler);
        
    private:
        VkImage mImage;
        ImageView mImageView;
        Sampler mSampler;
};

#endif