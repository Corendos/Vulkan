#ifndef OBJECTMANAGER
#define OBJECTMANAGER

#include <vector>

#include <vulkan/vulkan.hpp>

#include "renderer/Object.hpp"
#include "vulkan/VulkanContext.hpp"

class ObjectManager {
    public:
        ObjectManager() = default;
        ObjectManager(ObjectManager& other) = delete;

        ObjectManager& operator=(ObjectManager& other) = delete;

        void create(VulkanContext& context);
        void destroy();

        void addObject(Object& object);
        void removeObject(Object& object);

        VkDescriptorSetLayout getDescriptorSetLayout() const;

        void update();

    private:
        VulkanContext* mContext;

        std::vector<Object*> mObjects;

        VkBuffer mVertexBuffer;
        VkBuffer mIndexBuffer;

        VkDescriptorSetLayout mDescriptorSetLayout;

        bool mUpdateNeeded{false};
        bool mFirstAllocation{true};

        void createDescriptorPool();
};

#endif