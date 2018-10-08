#include "HelloTriangleApplication.hpp"
#include "utils.hpp"
#include "PhysicalDevicePicker.hpp"

#pragma region Public Methods
void HelloTriangleApplication::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}
#pragma endregion

#pragma region Private Methods
void HelloTriangleApplication::mainLoop() {
    while(!glfwWindowShouldClose(mWindow)) {
        glfwPollEvents();
        mVulkan.drawFrame();
    }
}

void HelloTriangleApplication::cleanup() {
    mVulkan.cleanup();
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

void HelloTriangleApplication::initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    mWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan API", nullptr, nullptr);
    glfwSetFramebufferSizeCallback(mWindow, windowResizedCallback);
    glfwSetWindowUserPointer(mWindow, this);
}

void HelloTriangleApplication::initVulkan() {
    mVulkan.init(mWindow, WIDTH, HEIGHT);
}

#pragma endregion

#pragma region Static Functions

void HelloTriangleApplication::windowResizedCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    app->mVulkan.requestResize();
}

#pragma endregion