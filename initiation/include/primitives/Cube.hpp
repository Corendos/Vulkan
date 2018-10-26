#ifndef CUBE
#define CUBE

#include <glm/glm.hpp>

#include "primitives/StaticObject.hpp"
#include "colors/Color.hpp"

class Cube : public StaticObject {
    public:
        Cube(float size, glm::vec3 position, Color3f color);
};

#endif