#include "renderer/Object.hpp"
#include "colors/Color.hpp"

unsigned long Object::nextId{0};

Object::Object(Object& other)
    : mVertices(other.mVertices), mIndices(other.mIndices), mUniqueId(nextId++) {}

Object::Object(std::vector<Vertex> vertices, std::vector<uint16_t> indices)
    : mVertices(vertices), mIndices(indices), mUniqueId(nextId++) {}

Object::Object(std::vector<Vertex>&& vertices, std::vector<uint16_t>&& indices)
    : mVertices(std::move(vertices)), mIndices(std::move(indices)), mUniqueId(nextId++) {}

Object& Object::operator=(Object& other) {
    mVertices = other.mVertices;
    mIndices = other.mIndices;
    mUniqueId = nextId++;
}

uint32_t Object::getVertexCount() const {
    return mVertices.size();
}

uint32_t Object::getIndexCount() const {
    return mIndices.size();
}

std::vector<Vertex>& Object::getVertices() {
    return mVertices;
}

std::vector<uint16_t>& Object::getIndices() {
    return mIndices;
}
VkDescriptorSet Object::getDescriptorSet() const {
    return mDescriptorSet;
}

void Object::setDescriptorSet(VkDescriptorSet descriptorSet) {
    mDescriptorSet = descriptorSet;
}

Object Object::temp(glm::vec3 position) {
    double size = 0.5;
    Color3f color = {0.3, 0.2, 0.1};
    std::vector<Vertex> vertices = {
        // Front face
        {{position.x - size, position.y - size, position.z - size}, {0.0, -1.0, 0.0}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{position.x + size, position.y - size, position.z - size}, {0.0, -1.0, 0.0}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{position.x + size, position.y - size, position.z + size}, {0.0, -1.0, 0.0}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{position.x - size, position.y - size, position.z + size}, {0.0, -1.0, 0.0}, {color.r, color.g, color.b}, {0.0f, 0.0f}},

        // Back Face
        {{position.x + size, position.y + size, position.z - size}, {0.0, 1.0, 0.0}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{position.x - size, position.y + size, position.z - size}, {0.0, 1.0, 0.0}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{position.x - size, position.y + size, position.z + size}, {0.0, 1.0, 0.0}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{position.x + size, position.y + size, position.z + size}, {0.0, 1.0, 0.0}, {color.r, color.g, color.b}, {0.0f, 0.0f}},
        
        // Top Face
        {{position.x - size, position.y - size, position.z + size}, {0.0, 0.0, 1.0}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{position.x + size, position.y - size, position.z + size}, {0.0, 0.0, 1.0}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{position.x + size, position.y + size, position.z + size}, {0.0, 0.0, 1.0}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{position.x - size, position.y + size, position.z + size}, {0.0, 0.0, 1.0}, {color.r, color.g, color.b}, {0.0f, 0.0f}},
        
        // Bottom Face
        {{position.x - size, position.y + size, position.z - size}, {0.0, 0.0, -1.0}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{position.x + size, position.y + size, position.z - size}, {0.0, 0.0, -1.0}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{position.x + size, position.y - size, position.z - size}, {0.0, 0.0, -1.0}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{position.x - size, position.y - size, position.z - size}, {0.0, 0.0, -1.0}, {color.r, color.g, color.b}, {0.0f, 0.0f}},
        
        // Right Face
        {{position.x + size, position.y - size, position.z - size}, {1.0, 0.0, 0.0}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{position.x + size, position.y + size, position.z - size}, {1.0, 0.0, 0.0}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{position.x + size, position.y + size, position.z + size}, {1.0, 0.0, 0.0}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{position.x + size, position.y - size, position.z + size}, {1.0, 0.0, 0.0}, {color.r, color.g, color.b}, {0.0f, 0.0f}},
        
        // Left Face
        {{position.x - size, position.y + size, position.z - size}, {-1.0, 0.0, 0.0}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{position.x - size, position.y - size, position.z - size}, {-1.0, 0.0, 0.0}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{position.x - size, position.y - size, position.z + size}, {-1.0, 0.0, 0.0}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{position.x - size, position.y + size, position.z + size}, {-1.0, 0.0, 0.0}, {color.r, color.g, color.b}, {0.0f, 0.0f}}
    };

    std::vector<uint16_t> indices = {
        0, 1, 2, 0, 2, 3,
        4, 5, 6, 4, 6, 7,
        8, 9, 10, 8, 10, 11,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 21, 22, 20, 22, 23
    };
    return Object(vertices, indices);
}