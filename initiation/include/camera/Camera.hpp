#ifndef CAMERA
#define CAMERA

#include <glm/glm.hpp>

class Camera {
    public:
        void update(double deltaPitch, double deltaYaw);
        glm::mat4 getView() const;

    private:
        glm::mat4 mView;

        double mPitch;
        double mYaw;
};

#endif