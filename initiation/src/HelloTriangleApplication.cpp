#include <chrono>
#include <thread>

#include "HelloTriangleApplication.hpp"
#include "utils.hpp"

void HelloTriangleApplication::run() {
    init();
    mainLoop();
    cleanup();
}

void HelloTriangleApplication::mainLoop() {
    while(!glfwWindowShouldClose(mWindow)) {
        auto startTime = std::chrono::high_resolution_clock::now();
        glfwPollEvents();
        
        if (mInput.getMouse().button[MouseButton::Left].pressed) {
            double deltaYaw = mInput.getMouse().delta.x * -0.5;
            double deltaPitch = mInput.getMouse().delta.y * 0.5;

            mCamera.update(deltaPitch, deltaYaw);
        }
        mVulkan.drawFrame();
        auto endTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration<double>(endTime - startTime);
        auto target = std::chrono::duration<double>(1.0 / TARGET_FPS);
        if (elapsed < target) {
            std::this_thread::sleep_for(target - elapsed);
        }
    }
    mInput.stop();
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
    mInput.start();

    mVulkan.setCamera(mCamera);
    mCamera.setExtent({WIDTH, HEIGHT});
    mCamera.setFov(70);
    mVulkan.init(mWindow, WIDTH, HEIGHT);
}

void HelloTriangleApplication::windowResizedCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    app->mVulkan.requestResize();
}

void HelloTriangleApplication::mousePosCallback(GLFWwindow* window, double xPos, double yPos) {
    auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    //app->mInput.updateMousePosition(xPos, yPos);
}

void HelloTriangleApplication::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    //app->mInput.updateMouseState(button, action, mods);
}
