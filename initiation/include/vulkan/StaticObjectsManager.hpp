#ifndef STATICOBJECTSMANAGER
#define STATICOBJECTSMANAGER

#include <vector>

#include <vulkan/vulkan.h>

#include "primitives/StaticObject.hpp"

class Vulkan;

class StaticObjectsManager {
    public:
        void create(Vulkan& vulkan);
        void destroy();
        void addStaticObject(StaticObject& staticObject);

        VkBuffer getUniformBuffer() const;
        VkBuffer getVertexBuffer() const;
        VkBuffer getindexBuffer() const;

    private:
        VkBuffer mVertexBuffer;
        VkBuffer mIndexBuffer;
        VkBuffer mUniformBuffer;

        std::vector<StaticObject> mStaticObjects;
        std::vector<Vertex> mVertices;
        std::vector<uint16_t> mIndices;

        Vulkan* mVulkan;

        void createVertexBuffer();
        void createIndexBuffer();
        void createUniformBuffer();
};

#endif