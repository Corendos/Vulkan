#ifndef HELLOTRIANGLEAPPLICATION
#define HELLOTRIANGLEAPPLICATION

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <optional>
#include <set>
#include <fstream>

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan/Vulkan.hpp"
#include "inputs/Input.hpp"
#include "utils.hpp"
#include "BasicLogger.hpp"

class HelloTriangleApplication {
    public:
        void run();

    private:
        GLFWwindow* mWindow;                                // Window handler
        Vulkan mVulkan;
        Input mInput;

        Camera mCamera;

        BasicLogger mOutLogger{"../out.log"};
        BasicLogger mErrLogger{"../err.log"};

        const unsigned int WIDTH{800};
        const unsigned int HEIGHT{600};
        const double TARGET_FPS{60.0};

        void init();
        void mainLoop();
        void drawFrame();
        void cleanup();

        static void windowResizedCallback(GLFWwindow* window, int width, int height);
        static void mousePosCallback(GLFWwindow* window, double xPos, double yPos);
        static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
};

#endif