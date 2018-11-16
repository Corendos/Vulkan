#ifndef INPUT
#define INPUT

#include <mutex>

#include <GLFW/glfw3.h>

#include "inputs/Mouse.hpp"

class Input {
    public:
        void attachWindow(GLFWwindow* mWindow);

        void updateMousePosition(double xPos, double yPos);
        void updateMouseState(int button, int action, int mods);

        void update();
        void start();
        void run();
        void stop();

        Mouse getMouse();

    private:
        GLFWwindow* mWindow;

        Mouse mMouse;
        std::mutex mMouseMutex;
        const double UPDATE_FREQUENCY{100.0};
        bool mShouldStop{false};

        std::thread mThread;

        void updateMouseButton(int button, int state);
};

#endif