#include "Transform.hpp"

Transform::Transform() {
    setPosition({0.0, 0.0, 0.0});
    setScale({1.0, 1.0, 1.0});
    setRotation({0.0, 0.0, 0.0});
}

void Transform::setPosition(glm::vec3 position) {
    mPosition = position;
}

void Transform::setScale(glm::vec3 scale) {
    mScale = scale;
}

void Transform::setRotation(glm::vec3 rotation) {
    mRotation = rotation;
    mRotationQuaternion = glm::quat(mRotation);
}

void Transform::move(glm::vec3 dP) {
    mPosition += dP;
}

void Transform::scale(glm::vec3 dScale) {
    mScale.x *= dScale.x;
    mScale.y *= dScale.y;
    mScale.z *= dScale.z;
}

void Transform::rotate(float angle, glm::vec3 axis) {
    glm::rotate(mRotationQuaternion, angle, axis);
    mRotation = glm::eulerAngles(mRotationQuaternion);
}

glm::mat4 Transform::getMatrix() {
    glm::mat4 model(1.0f);
    model = glm::translate(model, mPosition);
    model *= glm::mat4_cast(mRotationQuaternion);
    model = glm::scale(model, mScale);
    return model;
}