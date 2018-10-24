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

class StaticObjectsManager {
    public:
        void create(VkDevice device, MemoryManager& manager, CommandPool& commandPool, VkQueue queue);
        void destroy(VkDevice device);
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

        MemoryManager* mMemoryManager;

        void createVertexBuffer(VkDevice device, CommandPool& commandPool, VkQueue queue);
        void createIndexBuffer(VkDevice device, CommandPool& commandPool, VkQueue queue);
        void createUniformBuffer(VkDevice device);
};

#endif