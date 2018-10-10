#include <vector>
#include <array>

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};

        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDecscriptions = {};

        attributeDecscriptions[0].binding = 0;
        attributeDecscriptions[0].location = 0;
        attributeDecscriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDecscriptions[0].offset = offsetof(Vertex, pos);

        attributeDecscriptions[1].binding = 0;
        attributeDecscriptions[1].location = 1;
        attributeDecscriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDecscriptions[1].offset = offsetof(Vertex, color);

        return attributeDecscriptions;
    }
};

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
};