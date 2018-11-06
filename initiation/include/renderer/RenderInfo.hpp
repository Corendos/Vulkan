#ifndef RENDERINFO
#define RENDERINFO

#include <glm/glm.hpp>

struct RenderInfo {
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 cameraPosition;
    glm::vec4 lightPosition;
};

#endif