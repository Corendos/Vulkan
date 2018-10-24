#ifndef STATICOBJECT
#define STATICOBJECT

#include <vector>

#include "vulkan/Vertex.hpp"

class StaticObject {
    public:
        StaticObject(std::vector<Vertex> vertices, std::vector<uint16_t> indices);

        std::vector<Vertex>& getVertices();
        std::vector<uint16_t>& getIndices();

    private:
        std::vector<Vertex> mVertices;
        std::vector<uint16_t> mIndices;
};

#endif