#ifndef TEXTUREDCUBE
#define TEXTUREDCUBE

#include <glm/glm.hpp>

#include "primitives/StaticObject.hpp"
#include "colors/Color.hpp"
#include "vulkan/Image.hpp"

class TexturedCube : public StaticObject {
    public:
        TexturedCube(float size, glm::vec3 position, Color3f color);
};

#endif