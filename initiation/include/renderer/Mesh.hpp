#ifndef MESH
#define MESH

#include <vector>

#include "vulkan/Vertex.hpp"

class Mesh {
    public:
        Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
        Mesh(Mesh& other) = default;
        Mesh(Mesh&& other);

        Mesh& operator=(Mesh& other) = default;
        Mesh& operator=(Mesh&& other);
    private:
        std::vector<Vertex> mVertices;
        std::vector<uint32_t> mIndices;
};

#endif