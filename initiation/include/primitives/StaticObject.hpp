#ifndef STATICOBJECT
#define STATICOBJECT

#include <vector>

#include "vulkan/Vertex.hpp"
#include "vulkan/Image.hpp"

class StaticObject {
    public:
        StaticObject();
        StaticObject(std::vector<Vertex> vertices, std::vector<uint16_t> indices);

        std::vector<Vertex>& getVertices();
        std::vector<uint16_t>& getIndices();
        void setTexture(Image& image);
        Image& getTexture();

    protected:
        std::vector<Vertex> mVertices;
        std::vector<uint16_t> mIndices;
        Image* mTexture;
};

#endif