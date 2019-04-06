#ifndef TEXTUREMANAGER
#define TEXTUREMANAGER

#include <map>
#include <string>

#include "vulkan/VulkanContext.hpp"
#include "resources/Texture.hpp"

class TextureManager {
    public:
        void create(VulkanContext& context);
        void destroy();

        Texture& load(std::string name, std::string filename);
        Texture& getTexture(std::string name);

    private:
        VulkanContext* mContext;
        std::map<std::string, Texture> mTextures;

        vk::Buffer _loadToStaging(std::string& filename,
                                uint32_t& width,
                                uint32_t& height);
        vk::Image _createImage(std::string& filename);
        vk::ImageView _createImageView(vk::Image& image);
        vk::Sampler _createSampler();
};

#endif