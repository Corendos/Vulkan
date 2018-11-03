#include "primitives/StaticObject.hpp"

StaticObject::StaticObject(){}

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

Image& StaticObject::getTexture() {
    return *mTexture;
}

void StaticObject::setTexture(Image& image) {
    mTexture = &image;
}