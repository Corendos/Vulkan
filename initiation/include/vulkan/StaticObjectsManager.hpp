#ifndef STATICOBJECTSMANAGER
#define STATICOBJECTSMANAGER

#include <vector>

#include <vulkan/vulkan.h>

#include "primitives/StaticObject.hpp"
#include "camera/Camera.hpp"

class Vulkan;

class StaticObjectsManager {
    public:
        void create(Vulkan& vulkan);
        void destroy();
        void addStaticObject(StaticObject& staticObject);

        void update(Camera& camera);

        VkBuffer getUniformBuffer() const;
        VkBuffer getVertexBuffer() const;
        VkBuffer getIndexBuffer() const;
        uint32_t getIndiceCount();

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