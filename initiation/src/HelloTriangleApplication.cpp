#include <chrono>
#include <thread>

#include "HelloTriangleApplication.hpp"
#include "utils.hpp"
#include "primitives/TexturedCube.hpp"

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

        mRenderer.update();
        mRenderer.render();

        auto endTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration<double>(endTime - startTime);
        auto target = std::chrono::duration<double>(1.0 / TARGET_FPS);
        if (elapsed < target) {
            std::this_thread::sleep_for(target - elapsed);
        }
    }
}

void HelloTriangleApplication::cleanup() {
    vkDeviceWaitIdle(mContext.getDevice());
    mInput.stop();
    mTexture.destroy(mContext.getDevice(), mContext.getMemoryManager());
    mRenderer.destroy();
    mContext.destroy();
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
    
    mCamera.setExtent({WIDTH, HEIGHT});
    mCamera.setFov(70);
    mContext.create(mWindow);

    mTexture.loadFromFile(std::string(ROOT_PATH) + std::string("textures/dirt.png"),
                          mContext.getDevice(), mContext.getMemoryManager());
    mTexture.create(mContext.getDevice(),
                    mContext.getMemoryManager(),
                    mContext.getCommandPool(),
                    mContext.getGraphicsQueue());

    TexturedCube cube{0.5f, {0.0, 0.0, 0.0}, {1.0f, 1.0f, 1.0f}};
    cube.setTexture(mTexture);
    mRenderer.getStaticObjectManager().addStaticObject(cube);

    mRenderer.create(mContext);
    mRenderer.setCamera(mCamera);
}

void HelloTriangleApplication::windowResizedCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    app->mRenderer.recreate();
}

void HelloTriangleApplication::mousePosCallback(GLFWwindow* window, double xPos, double yPos) {
    auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    //app->mInput.updateMousePosition(xPos, yPos);
}

void HelloTriangleApplication::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    //app->mInput.updateMouseState(button, action, mods);
}
