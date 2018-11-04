#ifndef STATICOBJECTSMANAGER
#define STATICOBJECTSMANAGER

#include <vector>

#include <vulkan/vulkan.h>

#include "primitives/StaticObject.hpp"
#include "vulkan/VulkanContext.hpp"
#include "camera/Camera.hpp"

class Renderer;

struct StaticObjectInfo {
    StaticObject object;
    uint16_t indicesOffset;
    uint32_t indiceCount;
};

class StaticObjectsManager {
    public:
        void create(VulkanContext& context, Renderer& renderer);
        void destroy();
        void addStaticObject(StaticObject staticObject);

        VkBuffer getVertexBuffer() const;
        VkBuffer getIndexBuffer() const;
        uint32_t getIndiceCount() const;
        std::vector<StaticObjectInfo>& getObjectInfos();
        StaticObjectInfo& getObjectInfo(uint32_t index);
        std::vector<VkDescriptorSet>& getDescriptors();
        VkDescriptorSet getDescriptor(uint32_t index);

    private:
        VkBuffer mVertexBuffer;
        VkBuffer mIndexBuffer;
        std::vector<VkDescriptorSet> mDescriptorSets;

        std::vector<StaticObjectInfo> mStaticObjectsInfo;
        std::vector<Vertex> mVertices;
        std::vector<uint16_t> mIndices;

        Renderer* mRenderer;
        VulkanContext* mContext;

        void createVertexBuffer();
        void createIndexBuffer();
        void createDescriptorSets(VkDevice device,
                                  VkDescriptorPool descriptorPool,
                                  VkDescriptorSetLayout descriptorSetLayout);
};

#endif