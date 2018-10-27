#ifndef INPUT
#define INPUT

#include <GLFW/glfw3.h>

#include "inputs/Mouse.hpp"

class Input {
    public:
        void attachWindow(GLFWwindow* mWindow);

        void updateMousePosition(double xPos, double yPos);
        void updateMouseState(int button, int action, int mods);

        void update();

        Mouse& getMouse();

    private:
        GLFWwindow* mWindow;

        Mouse mMouse;

        void updateMouseButton(int button, int state);
};

#endif