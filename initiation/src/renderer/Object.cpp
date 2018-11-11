#include "renderer/Object.hpp"
#include "colors/Color.hpp"

unsigned long Object::nextId{0};

Object::Object(Object& other)
    : mVertices(other.mVertices), mIndices(other.mIndices), mUniqueId(nextId++),
    mTransform(other.mTransform) {}

Object::Object(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
    : mVertices(vertices), mIndices(indices), mUniqueId(nextId++) {}

Object::Object(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices)
    : mVertices(std::move(vertices)), mIndices(std::move(indices)), mUniqueId(nextId++) {}

Object& Object::operator=(Object& other) {
    mVertices = other.mVertices;
    mIndices = other.mIndices;
    mUniqueId = nextId++;
    mTransform = other.mTransform;
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

std::vector<uint32_t>& Object::getIndices() {
    return mIndices;
}

Texture& Object::getTexture() {
    return *mTexture;
}

VkDescriptorSet Object::getDescriptorSet() const {
    return mDescriptorSet;
}

void Object::setDescriptorSet(VkDescriptorSet descriptorSet) {
    mDescriptorSet = descriptorSet;
}

void Object::setTexture(Texture& texture) {
    mTexture = &texture;
}

Transform& Object::getTransform() {
    return mTransform;
}

Object Object::temp(glm::vec3 position) {
    double size = 0.5;
    Color3f color = {0.3, 0.2, 0.1};
    std::vector<Vertex> vertices = {
        // Front face
        {{ -size,  -size,  -size}, {0.0, -1.0, 0.0}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{ +size,  -size,  -size}, {0.0, -1.0, 0.0}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{ +size,  -size,  +size}, {0.0, -1.0, 0.0}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{ -size,  -size,  +size}, {0.0, -1.0, 0.0}, {color.r, color.g, color.b}, {0.0f, 0.0f}},

        // Back Face
        {{ +size,  +size,  -size}, {0.0, 1.0, 0.0}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{ -size,  +size,  -size}, {0.0, 1.0, 0.0}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{ -size,  +size,  +size}, {0.0, 1.0, 0.0}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{ +size,  +size,  +size}, {0.0, 1.0, 0.0}, {color.r, color.g, color.b}, {0.0f, 0.0f}},
        
        // Top Face
        {{ -size,  -size,  +size}, {0.0, 0.0, 1.0}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{ +size,  -size,  +size}, {0.0, 0.0, 1.0}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{ +size,  +size,  +size}, {0.0, 0.0, 1.0}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{ -size,  +size,  +size}, {0.0, 0.0, 1.0}, {color.r, color.g, color.b}, {0.0f, 0.0f}},
        
        // Bottom Face
        {{ -size,  +size,  -size}, {0.0, 0.0, -1.0}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{ +size,  +size,  -size}, {0.0, 0.0, -1.0}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{ +size,  -size,  -size}, {0.0, 0.0, -1.0}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{ -size,  -size,  -size}, {0.0, 0.0, -1.0}, {color.r, color.g, color.b}, {0.0f, 0.0f}},
        
        // Right Face
        {{ +size,  -size,  -size}, {1.0, 0.0, 0.0}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{ +size,  +size,  -size}, {1.0, 0.0, 0.0}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{ +size,  +size,  +size}, {1.0, 0.0, 0.0}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{ +size,  -size,  +size}, {1.0, 0.0, 0.0}, {color.r, color.g, color.b}, {0.0f, 0.0f}},
        
        // Left Face
        {{ -size,  +size,  -size}, {-1.0, 0.0, 0.0}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{ -size,  -size,  -size}, {-1.0, 0.0, 0.0}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{ -size,  -size,  +size}, {-1.0, 0.0, 0.0}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{ -size,  +size,  +size}, {-1.0, 0.0, 0.0}, {color.r, color.g, color.b}, {0.0f, 0.0f}}
    };

    std::vector<uint32_t> indices = {
        0, 1, 2, 0, 2, 3,
        4, 5, 6, 4, 6, 7,
        8, 9, 10, 8, 10, 11,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 21, 22, 20, 22, 23
    };
    Object o(vertices, indices);
    o.getTransform().setPosition(position);
    return o;
}