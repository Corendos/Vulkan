#include "primitives/StaticObject.hpp"

StaticObject::StaticObject(std::vector<Vertex> vertices, std::vector<uint16_t> indices) {
    mVertices = vertices;
    mIndices = indices;
}

std::vector<Vertex>& StaticObject::getVertices() {
    return mVertices;
}

std::vector<uint16_t>& StaticObject::getIndices() {
    return mIndices;
}