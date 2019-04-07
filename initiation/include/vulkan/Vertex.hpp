#ifndef VERTEX
#define VERTEX

#include <vector>
#include <array>

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normals;
    glm::vec3 color;
    glm::vec2 texCoord;

    static vk::VertexInputBindingDescription getBindingDescription() {
        vk::VertexInputBindingDescription bindingDescription;
        bindingDescription.setBinding(0);
        bindingDescription.setStride(sizeof(Vertex));
        bindingDescription.setInputRate(vk::VertexInputRate::eVertex);

        return bindingDescription;
    }

    static std::array<vk::VertexInputAttributeDescription, 4> getAttributeDescriptions() {
        std::array<vk::VertexInputAttributeDescription, 4> attributeDescriptions;

        attributeDescriptions[0].setBinding(0) = 0;
        attributeDescriptions[0].setLocation(0) = 0;
        attributeDescriptions[0].setFormat(vk::Format::eR32G32B32Sfloat);
        attributeDescriptions[0].setOffset(offsetof(Vertex, pos));

        attributeDescriptions[1].setBinding(0) = 0;
        attributeDescriptions[1].setLocation(1) = 1;
        attributeDescriptions[1].setFormat(vk::Format::eR32G32B32Sfloat);
        attributeDescriptions[1].setOffset(offsetof(Vertex, normals));

        attributeDescriptions[2].setBinding(0) = 0;
        attributeDescriptions[2].setLocation(2) = 2;
        attributeDescriptions[2].setFormat(vk::Format::eR32G32B32Sfloat);
        attributeDescriptions[2].setOffset(offsetof(Vertex, color));

        attributeDescriptions[3].setBinding(0) = 0;
        attributeDescriptions[3].setLocation(3) = 3;
        attributeDescriptions[3].setFormat(vk::Format::eR32G32Sfloat);
        attributeDescriptions[3].setOffset(offsetof(Vertex, texCoord));

        return attributeDescriptions;
    }
};

#endif