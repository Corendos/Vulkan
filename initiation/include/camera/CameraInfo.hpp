#ifndef CAMERAINFO
#define CAMERAINFO

#include <glm/glm.hpp>

struct CameraInfo {
    glm::mat4 view;
    glm::mat4 proj;
};

#endif