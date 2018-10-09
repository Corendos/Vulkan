#include "HelloTriangleApplication.hpp"
#include "utils.hpp"
#include "PhysicalDevicePicker.hpp"

void HelloTriangleApplication::run() {
    init();
    mainLoop();
    cleanup();
}

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

void HelloTriangleApplication::init() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    mWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan API", nullptr, nullptr);
    glfwSetFramebufferSizeCallback(mWindow, windowResizedCallback);
    glfwSetWindowUserPointer(mWindow, this);

    mVulkan.init(mWindow, WIDTH, HEIGHT);
}

void HelloTriangleApplication::windowResizedCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    app->mVulkan.requestResize();
}