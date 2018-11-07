#ifndef OBJECTMANAGER
#define OBJECTMANAGER

#include <vector>

#include <vulkan/vulkan.hpp>

#include "renderer/Object.hpp"
#include "vulkan/VulkanContext.hpp"

class ObjectManager {
    public:
        ObjectManager(ObjectManager& other) = delete;

        ObjectManager& operator=(ObjectManager& other) = delete;

        void create(VulkanContext& context);
        void destroy();

        void addObject(Object& object);
        void removeObject(Object& object);

        void update();

    private:
        VulkanContext* mContext;

        std::vector<Object*> mObjects;

        VkBuffer mVertexBuffer;
        VkBuffer mIndexBuffer;

        bool mUpdateNeeded{false};
        bool mFirstAllocation{true};
};

#endif