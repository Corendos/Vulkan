#include "renderer/Object.hpp"

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
    return mVertices.size();
}

std::vector<Vertex>& Object::getVertices() {
    return mVertices;
}

std::vector<uint16_t>& Object::getIndices() {
    return mIndices;
}

Object Object::temp() {
    std::vector<Vertex> vertices = {
        {{0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, {0.0, 0.0}},
        {{1.0, 0.0, 0.0}, {0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, {1.0, 0.0}},
        {{0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, {0.0, 1.0}},
    };
    std::vector<uint16_t> indices = {0, 1, 2};
    return Object(vertices, indices);
}