#ifndef STATICOBJECTSMANAGER
#define STATICOBJECTSMANAGER

#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "primitives/StaticObject.hpp"
#include "memory/MemoryManager.hpp"
#include "vulkan/BufferHelper.hpp"
#include "vulkan/UniformBufferObject.hpp"
#include "utils.hpp"

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