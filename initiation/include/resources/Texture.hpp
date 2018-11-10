#ifndef TEXTURE
#define TEXTURE

#include <vulkan/vulkan.h>

class Texture {
    public:
    private:
        VkImage mImage;
        VkImageView mImageView;
        VkSampler mSampler;
};

#endif