#ifndef CUBE
#define CUBE

#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "vulkan/Vertex.hpp"
#include "vulkan/BufferHelper.hpp"
#include "vulkan/UniformBufferObject.hpp"
#include "vulkan/SwapChain.hpp"
#include "memory/MemoryManager.hpp"
#include "utils.hpp"

class Cube {
    public:
        Cube(float size = 1.0f);
        void create(MemoryManager& manager,
                    VkDevice device,
                    CommandPool& commandPool,
                    VkQueue queue,
                    SwapChain& swapChain,
                    VkDescriptorPool descriptorPool,
                    VkDescriptorSetLayout layout,
                    VkImageView imageView,
                    VkSampler sampler);
        void destroy(MemoryManager& memoryManager);
        VkBuffer getVertexBuffer();
        VkBuffer getIndicesBuffer();
        VkDescriptorSet getDescriptorSet();

    private:
        float mSize{1.0f};
        VkBuffer mVertexBuffer;
        VkBuffer mIndicesBuffer;
        VkBuffer mUniformBuffer;
        VkDescriptorSet mDescriptor;

        std::array<Vertex, 8> mVertices;
        const std::array<uint16_t, 36> mIndices = {
            1, 2, 3, 1, 3, 4,
            5, 1, 4, 5, 4, 8,
            2, 6, 7, 2, 7, 3,
            5, 6, 2, 5, 2, 1,
            4, 3, 7, 4, 7, 8,
            6, 5, 8, 6, 8, 7
        };

        void createDescriptor(VkDevice device,
                              VkDescriptorPool descriptorPool,
                              VkDescriptorSetLayout layout,
                              VkImageView imageView,
                              VkSampler sampler);
};

#endif