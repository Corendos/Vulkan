#include <algorithm>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/camera/Camera.hpp"
#include "utils.hpp"

Camera::Camera() : mYaw(0.0), mPitch(0.0) {
    mPosition = glm::vec3(mDistance * std::sin(glm::radians(mYaw)) * std::cos(glm::radians(mPitch)),
                          mDistance * -std::cos(glm::radians(mYaw)) * std::cos(glm::radians(mPitch)),
                          mDistance * std::sin(glm::radians(mPitch)));

    mView = glm::lookAt(mPosition,
                        glm::vec3(0.0, 0.0, 0.0),
                        glm::vec3(0.0, 0.0, 1.0));
}

void Camera::update(double deltaPitch, double deltaYaw) {
    mPitch += deltaPitch;
    mPitch = std::clamp(mPitch, -90.0, 90.0);

    mYaw += deltaYaw;
    if (mYaw > 360.0) {
        mYaw -= 360.0;
    } else if (mYaw < -360.0) {
        mYaw += 360.0;
    }

    mPosition = glm::vec3(mDistance * std::sin(glm::radians(mYaw)) * std::cos(glm::radians(mPitch)),
                          mDistance * -std::cos(glm::radians(mYaw)) * std::cos(glm::radians(mPitch)),
                          mDistance * std::sin(glm::radians(mPitch)));

    mView = glm::lookAt(mPosition,
                        glm::vec3(0.0, 0.0, 0.0),
                        glm::vec3(0.0, 0.0, 1.0));
}

void Camera::setExtent(vk::Extent2D extent) {
    mExtent = extent;
    mProj = glm::vulkanPerspective(glm::radians(mFov), (double)mExtent.width / (double)mExtent.height, 0.1, 100.0);
}

void Camera::setFov(double fov) {
    mFov = fov;
    mProj = glm::vulkanPerspective(glm::radians(mFov), (double)mExtent.width / (double)mExtent.height, 0.1, 100.0);
}

glm::mat4 Camera::getView() const {
    return mView;
}

glm::mat4 Camera::getProj() const {
    return mProj;
}

glm::vec3 Camera::getPosition() const {
    return mPosition;
}