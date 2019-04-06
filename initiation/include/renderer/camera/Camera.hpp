#ifndef CAMERA
#define CAMERA

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

class Camera {
    public:
        Camera();
        void update(double deltaPitch, double deltaYaw);
        void setExtent(vk::Extent2D extent);
        void setFov(double fov);
        glm::mat4 getView() const;
        glm::mat4 getProj() const;
        glm::vec3 getPosition() const;

    private:
        glm::mat4 mView;
        glm::mat4 mProj;
        glm::vec3 mPosition;

        vk::Extent2D mExtent;
        double mFov;

        double mPitch;
        double mYaw;
        const double mDistance{5.0};
};

#endif