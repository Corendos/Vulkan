#ifndef CAMERAINFO
#define CAMERAINFO

#include <glm/glm.hpp>
#include "renderer/Light.hpp"

struct CameraInfo {
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 position;
    glm::vec4 light;
};

#endif