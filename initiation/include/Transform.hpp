#ifndef TRANSFORM
#define TRANSFORM

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transform {
    public:
        Transform();

        void setPosition(glm::vec3 position);
        void setScale(glm::vec3 scale);
        void setRotation(glm::vec3 mRotation);

        void move(glm::vec3 dP);
        void scale(glm::vec3 dScale);
        void rotate(float angle, glm::vec3 axis);

        glm::mat4 getMatrix();

    private:
        glm::vec3 mPosition;
        glm::vec3 mScale;
        glm::vec3 mRotation;

        glm::quat mRotationQuaternion;
};

#endif