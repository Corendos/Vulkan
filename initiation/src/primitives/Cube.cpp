#include "primitives/Cube.hpp"

Cube::Cube(float size, glm::vec3 position, Color3f color) {
    mVertices = {
        {{position.x + size, position.y + size, position.z + size}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{position.x - size, position.y + size, position.z + size}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{position.x - size, position.y - size, position.z + size}, {color.r, color.g, color.b}, {0.0f, 0.0f}},
        {{position.x + size, position.y - size, position.z + size}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{position.x + size, position.y + size, position.z - size}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{position.x - size, position.y + size, position.z - size}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{position.x - size, position.y - size, position.z - size}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{position.x + size, position.y - size, position.z - size}, {color.r, color.g, color.b}, {0.0f, 0.0f}}
    };
    mIndices = {
        0, 1, 2, 0, 2, 3,
        4, 0, 3, 4, 3, 7,
        1, 5, 6, 1, 6, 2,
        4, 5, 1, 4, 1, 0,
        3, 2, 6, 3, 6, 7,
        5, 4, 7, 5, 7, 6
    };
}