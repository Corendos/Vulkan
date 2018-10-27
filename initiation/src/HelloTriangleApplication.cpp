#include <chrono>

#include "HelloTriangleApplication.hpp"
#include "utils.hpp"

void HelloTriangleApplication::run() {
    init();
    mainLoop();
    cleanup();
}

void HelloTriangleApplication::mainLoop() {
    auto lastTime = std::chrono::high_resolution_clock::now();

    while(!glfwWindowShouldClose(mWindow)) {
        glfwPollEvents();
        mInput.update();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        if (mInput.getMouse().button[MouseButton::Left].pressed) {
            double deltaYaw = mInput.getMouse().delta.x * -0.5;
            double deltaPitch = mInput.getMouse().delta.y * 0.5;

            mCamera.update(deltaPitch, deltaYaw);
        }
        mVulkan.drawFrame();
    }
}

void HelloTriangleApplication::cleanup() {
    mVulkan.cleanup();
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

void HelloTriangleApplication::init() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    mWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan API", nullptr, nullptr);
    glfwSetFramebufferSizeCallback(mWindow, windowResizedCallback);
    glfwSetCursorPosCallback(mWindow, mousePosCallback);
    glfwSetMouseButtonCallback(mWindow, mouseButtonCallback);
    glfwSetWindowUserPointer(mWindow, this);

    mInput.attachWindow(mWindow);

    mVulkan.init(mWindow, WIDTH, HEIGHT);
    mVulkan.setCamera(mCamera);
    mCamera.setFov(70);
    mCamera.setExtent({WIDTH, HEIGHT});
}

void HelloTriangleApplication::windowResizedCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    app->mVulkan.requestResize();
}

void HelloTriangleApplication::mousePosCallback(GLFWwindow* window, double xPos, double yPos) {
    auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    app->mInput.updateMousePosition(xPos, yPos);
}

void HelloTriangleApplication::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    app->mInput.updateMouseState(button, action, mods);
}
