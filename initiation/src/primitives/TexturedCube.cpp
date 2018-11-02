#include "primitives/TexturedCube.hpp"

TexturedCube::TexturedCube(float size, glm::vec3 position, Color3f color) {
    mVertices = {
        // Front face
        {{position.x - size, position.y - size, position.z - size}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{position.x + size, position.y - size, position.z - size}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{position.x + size, position.y - size, position.z + size}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{position.x - size, position.y - size, position.z + size}, {color.r, color.g, color.b}, {0.0f, 0.0f}},

        // Back Face
        {{position.x + size, position.y + size, position.z - size}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{position.x - size, position.y + size, position.z - size}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{position.x - size, position.y + size, position.z + size}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{position.x + size, position.y + size, position.z + size}, {color.r, color.g, color.b}, {0.0f, 0.0f}},
        
        // Top Face
        {{position.x - size, position.y - size, position.z + size}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{position.x + size, position.y - size, position.z + size}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{position.x + size, position.y + size, position.z + size}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{position.x - size, position.y + size, position.z + size}, {color.r, color.g, color.b}, {0.0f, 0.0f}},
        
        // Bottom Face
        {{position.x - size, position.y + size, position.z - size}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{position.x + size, position.y + size, position.z - size}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{position.x + size, position.y - size, position.z - size}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{position.x - size, position.y - size, position.z - size}, {color.r, color.g, color.b}, {0.0f, 0.0f}},
        
        // Right Face
        {{position.x + size, position.y - size, position.z - size}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{position.x + size, position.y + size, position.z - size}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{position.x + size, position.y + size, position.z + size}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{position.x + size, position.y - size, position.z + size}, {color.r, color.g, color.b}, {0.0f, 0.0f}},
        
        // Left Face
        {{position.x - size, position.y + size, position.z - size}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{position.x - size, position.y - size, position.z - size}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{position.x - size, position.y - size, position.z + size}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{position.x - size, position.y + size, position.z + size}, {color.r, color.g, color.b}, {0.0f, 0.0f}}
    };

    mIndices = {
        0, 1, 2, 0, 2, 3,
        4, 5, 6, 4, 6, 7,
        8, 9, 10, 8, 10, 11,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 21, 22, 20, 22, 23
    };
}