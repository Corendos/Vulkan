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

#include "Vulkan.hpp"
#include "utils.hpp"
#include "BasicLogger.hpp"

class HelloTriangleApplication {
    public:
        void run();

    private:
        GLFWwindow* mWindow;                                // Window handler
        Vulkan mVulkan;

        BasicLogger mOutLogger{"../out.log"};
        BasicLogger mErrLogger{"../err.log"};

        const unsigned int WIDTH{800};
        const unsigned int HEIGHT{600};

        void initWindow();
        void initVulkan();
        void mainLoop();
        void drawFrame();
        void cleanup();

        static void windowResizedCallback(GLFWwindow* window, int width, int height);
};

#endif