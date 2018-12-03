#include "renderer/mesh/MeshHelper.hpp"
#include "colors/Color.hpp"

Mesh MeshHelper::createCube(double size) {
    Color3f color = {0.3, 0.2, 0.1};
    std::vector<Vertex> vertices = {
        // Front face
        {{ -size,  -size,  -size}, {0.0, -1.0, 0.0}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{ +size,  -size,  -size}, {0.0, -1.0, 0.0}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{ +size,  -size,  +size}, {0.0, -1.0, 0.0}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{ -size,  -size,  +size}, {0.0, -1.0, 0.0}, {color.r, color.g, color.b}, {0.0f, 0.0f}},

        // Back Face
        {{ +size,  +size,  -size}, {0.0, 1.0, 0.0}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{ -size,  +size,  -size}, {0.0, 1.0, 0.0}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{ -size,  +size,  +size}, {0.0, 1.0, 0.0}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{ +size,  +size,  +size}, {0.0, 1.0, 0.0}, {color.r, color.g, color.b}, {0.0f, 0.0f}},
        
        // Top Face
        {{ -size,  -size,  +size}, {0.0, 0.0, 1.0}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{ +size,  -size,  +size}, {0.0, 0.0, 1.0}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{ +size,  +size,  +size}, {0.0, 0.0, 1.0}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{ -size,  +size,  +size}, {0.0, 0.0, 1.0}, {color.r, color.g, color.b}, {0.0f, 0.0f}},
        
        // Bottom Face
        {{ -size,  +size,  -size}, {0.0, 0.0, -1.0}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{ +size,  +size,  -size}, {0.0, 0.0, -1.0}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{ +size,  -size,  -size}, {0.0, 0.0, -1.0}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{ -size,  -size,  -size}, {0.0, 0.0, -1.0}, {color.r, color.g, color.b}, {0.0f, 0.0f}},
        
        // Right Face
        {{ +size,  -size,  -size}, {1.0, 0.0, 0.0}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{ +size,  +size,  -size}, {1.0, 0.0, 0.0}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{ +size,  +size,  +size}, {1.0, 0.0, 0.0}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{ +size,  -size,  +size}, {1.0, 0.0, 0.0}, {color.r, color.g, color.b}, {0.0f, 0.0f}},
        
        // Left Face
        {{ -size,  +size,  -size}, {-1.0, 0.0, 0.0}, {color.r, color.g, color.b}, {0.0f, 1.0f}},
        {{ -size,  -size,  -size}, {-1.0, 0.0, 0.0}, {color.r, color.g, color.b}, {1.0f, 1.0f}},
        {{ -size,  -size,  +size}, {-1.0, 0.0, 0.0}, {color.r, color.g, color.b}, {1.0f, 0.0f}},
        {{ -size,  +size,  +size}, {-1.0, 0.0, 0.0}, {color.r, color.g, color.b}, {0.0f, 0.0f}}
    };

    std::vector<uint32_t> indices = {
        0, 1, 2, 0, 2, 3,
        4, 5, 6, 4, 6, 7,
        8, 9, 10, 8, 10, 11,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 21, 22, 20, 22, 23
    };

    return Mesh(vertices, indices);
}