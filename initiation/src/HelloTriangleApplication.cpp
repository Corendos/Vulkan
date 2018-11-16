#include <thread>

#include "HelloTriangleApplication.hpp"
#include "utils.hpp"

void HelloTriangleApplication::run() {
    init();
    mainLoop();
    cleanup();
}

void HelloTriangleApplication::mainLoop() {
    int i{0};
    uint32_t updateMean{0}, renderMean{0};

    auto updateEnd = std::chrono::high_resolution_clock::now();
    mFrameStartTime = std::chrono::high_resolution_clock::now();

    while(!glfwWindowShouldClose(mWindow)) {
        glfwPollEvents();

        processInputs();

        auto start = std::chrono::high_resolution_clock::now();
        double dt = std::chrono::duration<double>(start - updateEnd).count();
        mRenderer.update(dt);
        updateEnd = std::chrono::high_resolution_clock::now();
        mRenderer.render();
        auto end = std::chrono::high_resolution_clock::now();
        uint32_t updateDuration = std::chrono::duration_cast<std::chrono::microseconds>(updateEnd - start).count();
        uint32_t renderDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - updateEnd).count();
        
        updateMean += updateDuration;
        renderMean += renderDuration;
        i++;

        if (i % 10 == 0) {
            std::cout << "Update: " << (float)updateMean / 10.0f << "µs" << std::endl;
            std::cout << "Render: " << (float)renderMean / 10.0f << "µs" << std::endl;
            i = 0;
            updateMean = 0;
            renderMean = 0;
        }
        
        sleepUntilNextFrame();
    }
}

void HelloTriangleApplication::cleanup() {
    vkDeviceWaitIdle(mContext.getDevice());
    mInput.stop();
    mRenderer.destroy();
    mTextureManager.destroy();
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

    mLight = {{2.0, 2.0, 2.0}};

    mTextureManager.create(mContext);
    mTextureManager.load("diamond", std::string(ROOT_PATH) + std::string("textures/diamond.png"));
    mTextureManager.load("dirt", std::string(ROOT_PATH) + std::string("textures/dirt.png"));

    mRenderer.create(mContext, mTextureManager);
    mRenderer.setCamera(mCamera);
    mRenderer.setLight(mLight);
}

void HelloTriangleApplication::sleepUntilNextFrame() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration<double>(currentTime - mFrameStartTime);
    auto target = std::chrono::duration<double>(1.0 / TARGET_FPS);
    
    if (elapsed < target) {
        std::this_thread::sleep_for(target - elapsed);
    }

    mFrameStartTime = std::chrono::high_resolution_clock::now();
}

void HelloTriangleApplication::processInputs() {
    if (mInput.getMouse().button[MouseButton::Left].pressed) {
        double deltaYaw = mInput.getMouse().delta.x * -0.5;
        double deltaPitch = mInput.getMouse().delta.y * 0.5;

        mCamera.update(deltaPitch, deltaYaw);
    }
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
