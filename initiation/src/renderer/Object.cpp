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
