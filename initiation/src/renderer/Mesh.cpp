#include "renderer/Mesh.hpp"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices) :
    mVertices(vertices), mIndices(indices) {}

Mesh::Mesh(Mesh&& other) :
    mVertices(std::move(other.mVertices)), mIndices(std::move(other.mIndices)) {}

Mesh& Mesh::operator=(Mesh&& other) {
    mVertices = std::move(other.mVertices);
    mIndices = std::move(other.mIndices);
    return *this;
}
