#include <algorithm>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

#include "camera/Camera.hpp"

void Camera::update(double deltaPitch, double deltaYaw) {
    mPitch += deltaPitch;
    mPitch = std::clamp(mPitch, -90.0, 90.0);

    mYaw += deltaYaw;
    if (mYaw > 360.0) {
        mYaw -= 360.0;
    } else if (mYaw < -360.0) {
        mYaw += 360.0;
    }

    glm::vec3 eyes(4.0 * std::cos(glm::radians(mYaw)),
                   4.0 * std::sin(glm::radians(mYaw)),
                   4.0 * std::sin(glm::radians(mPitch)));

    mView = glm::lookAt(eyes,
                        glm::vec3(0.0, 0.0, 0.0),
                        glm::vec3(0.0, 0.0, 1.0));
}

glm::mat4 Camera::getView() const {
    return mView;
}