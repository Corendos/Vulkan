#ifndef MESH
#define MESH

#include <vector>

#include "vulkan/Vertex.hpp"
#include "resources/Texture.hpp"
#include "Transform.hpp"

class Mesh {
    public:
        Mesh() = default;
        Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
        Mesh(Mesh& other) = default;
        Mesh(Mesh&& other);

        Mesh& operator=(Mesh& other) = default;
        Mesh& operator=(Mesh&& other);

        Transform& getTransform();
        const std::vector<Vertex>& getVertices() const;
        const std::vector<uint32_t>& getIndices() const;

        void setTexture(Texture& texture);
    private:
        std::vector<Vertex> mVertices;
        std::vector<uint32_t> mIndices;

        Texture* mTexture;

        Transform mTransform;
};

#endif