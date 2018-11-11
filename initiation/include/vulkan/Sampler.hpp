#ifndef SAMPLER
#define SAMPLER

#include <stdexcept>
#include <vulkan/vulkan.h>

class Sampler {
    public:
        Sampler();

        void create(VkDevice device);
        void destroy(VkDevice device);

        VkSampler getHandler() const;

        void setFlags(VkSamplerCreateFlags flags);
        void setMagFilter(VkFilter magFilter);
        void setMinFilter(VkFilter minFilter);
        void setMipmapMode(VkSamplerMipmapMode mipmapMode);
        void setAddressModeU(VkSamplerAddressMode addressModeU);
        void setAddressModeV(VkSamplerAddressMode addressModeV);
        void setAddressModeW(VkSamplerAddressMode addressModeW);
        void setMipLodBias(float mipLodBias);
        void setAnisotropyEnable(VkBool32 anisotropyEnable);
        void setMaxAnisotropy(float maxAnisotropy);
        void setCompareEnable(VkBool32 compareEnable);
        void setCompareOp(VkCompareOp compareOp);
        void setMinLod(float minLod);
        void setMaxLod(float maxLod);
        void setBorderColor(VkBorderColor borderColor);
        void setUnnormalizedCoordinates(VkBool32 unnormalizedCoordinates);

    private:
        VkSampler mSampler;
        VkSamplerCreateInfo mInfo{};
};

#endif