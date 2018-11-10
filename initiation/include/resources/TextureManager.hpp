#ifndef TEXTUREMANAGER
#define TEXTUREMANAGER

#include "vulkan/VulkanContext.hpp"

class TextureManager {
    public:
        void create(VulkanContext& context);
        void destroy();

    private:
        VulkanContext* mContext;

};

#endif