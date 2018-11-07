#ifndef OBJECT
#define OBJECT

#include <vector>

#include <glm/glm.hpp>

#include "vulkan/Vertex.hpp"

class Object {
    public:
        Object(Object& other);
        Object(Object&& other) = default;
        Object(std::vector<Vertex> vertices, std::vector<uint16_t> indices);
        Object(std::vector<Vertex>&& vertices, std::vector<uint16_t>&& indices);

        Object& operator=(Object& other);
        Object& operator=(Object&& other) = default;

        uint32_t getVertexCount() const;
        uint32_t getIndexCount() const;

    private:
        std::vector<Vertex> mVertices;
        std::vector<uint16_t> mIndices;
        unsigned long mUniqueId;

        static unsigned long nextId;
};

#endif