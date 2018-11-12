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

        VkBuffer _loadToStaging(std::string& filename,
                                uint32_t& width,
                                uint32_t& height);
        Image _createImage(std::string& filename);
        ImageView _createImageView(Image& image);
        Sampler _createSampler();
};

#endif