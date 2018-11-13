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

        void updateBuffers();
        void updateUniformBuffer();

    private:
        VulkanContext* mContext;

        std::vector<Object*> mObjects;
        VkBuffer mVertexBuffer;
        VkBuffer mIndexBuffer;
        VkBuffer mModelMatrixBuffer;

        std::map<Object*, uint32_t> mObjectDynamicOffset;

        VkDescriptorSetLayout mDescriptorSetLayout;
        std::vector<DescriptorSetHandler> mDescriptorSetHandlers;

        uint32_t mObjectsCount{0};

        bool mUpdateNeeded{false};
        bool mFirstAllocation{true};

        void createDescriptorSetLayout();
        void createAndAllocateUniformBuffer();
        void allocateDescriptorSets();

        static constexpr uint32_t mMaxObjectCount{1024};
};

#endif