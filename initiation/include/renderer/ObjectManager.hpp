#ifndef OBJECTMANAGER
#define OBJECTMANAGER

#include <vector>
#include <map>

#include <vulkan/vulkan.hpp>

#include "renderer/Object.hpp"
#include "vulkan/VulkanContext.hpp"
#include "vulkan/Image.hpp"

struct DescriptorSetHandler {
    VkDescriptorSet descriptorSet;
    bool used{false};
};

class ObjectManager {
    public:
        ObjectManager() = default;
        ObjectManager(ObjectManager& other) = delete;

        ObjectManager& operator=(ObjectManager& other) = delete;

        void create(VulkanContext& context);
        void destroy();

        void addObject(Object& object);
        void removeObject(Object& object);

        void render(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);

        VkDescriptorSetLayout getDescriptorSetLayout() const;
        std::vector<Object*>& getObjects();

        void update();

    private:
        VulkanContext* mContext;

        std::vector<Object*> mObjects;
        VkBuffer mVertexBuffer;
        VkBuffer mIndexBuffer;

        Image mImage;

        VkDescriptorSetLayout mDescriptorSetLayout;
        std::vector<DescriptorSetHandler> mDescriptorSetHandlers;

        const uint32_t mInitialDescriptorSetsCount{50000};

        bool mUpdateNeeded{false};
        bool mFirstAllocation{true};

        void createDescriptorSetLayout();
        void allocateDescriptorSets();
};

#endif