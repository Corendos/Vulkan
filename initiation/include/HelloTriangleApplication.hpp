#ifndef HELLOTRIANGLEAPPLICATION
#define HELLOTRIANGLEAPPLICATION

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <set>
#include <fstream>
#include <chrono>
#include <thread>

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan/VulkanContext.hpp"
#include "vulkan/Image.hpp"
#include "resources/TextureManager.hpp"
#include "camera/Camera.hpp"
#include "renderer/Renderer.hpp"
#include "renderer/Light.hpp"
#include "renderer/ObjectManager.hpp"
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
        TextureManager mTextureManager;
        ObjectManager mObjectManager;
        std::vector<std::unique_ptr<Object>> mObjects;

        BasicLogger mOutLogger{"../out.log"};
        BasicLogger mErrLogger{"../err.log"};

        const unsigned int WIDTH{1920};
        const unsigned int HEIGHT{1080};
        const double TARGET_FPS{60.0};

        std::chrono::time_point<std::chrono::high_resolution_clock> mFrameStartTime;

        void init();
        void mainLoop();
        void drawFrame();
        void cleanup();

        void sleepUntilNextFrame();
        void processInputs();

        static void windowResizedCallback(GLFWwindow* window, int width, int height);
        static void mousePosCallback(GLFWwindow* window, double xPos, double yPos);
        static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
};

#endif