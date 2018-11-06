#ifndef HELLOTRIANGLEAPPLICATION
#define HELLOTRIANGLEAPPLICATION

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <set>
#include <fstream>
#include <thread>

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan/VulkanContext.hpp"
#include "vulkan/Image.hpp"
#include "camera/Camera.hpp"
#include "renderer/Renderer.hpp"
#include "renderer/Light.hpp"
#include "inputs/Input.hpp"
#include "utils.hpp"
#include "BasicLogger.hpp"

class HelloTriangleApplication {
    public:
        void run();

    private:
        GLFWwindow* mWindow;                                // Window handler
        VulkanContext mContext;
        Input mInput;
        std::thread mInputThread;

        Renderer mRenderer;
        Camera mCamera;
        Light mLight;
        Image mTexture;

        BasicLogger mOutLogger{"../out.log"};
        BasicLogger mErrLogger{"../err.log"};

        const unsigned int WIDTH{1920};
        const unsigned int HEIGHT{1080};
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