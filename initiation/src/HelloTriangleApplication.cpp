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

        if (mInput.getMouse().button[MouseButton::Right].down) {
            Object o = Object::temp({0, 0, 0});
            o.setTexture(mTextureManager.getTexture("diamond"));
            mObjects.push_back(std::make_unique<Object>(std::move(o)));
            mObjectManager.addObject(*mObjects.back());
        }

        auto start = std::chrono::high_resolution_clock::now();
        double dt = std::chrono::duration<double>(start - updateEnd).count();
        for (auto& o : mObjects) {
            o->getTransform().rotate(3.14159265 * dt, glm::vec3(0.0, 0.0, 1.0));
        }
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
    mObjectManager.destroy();
    mMeshManager.destroy();
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

    mObjectManager.create(mContext);
    int size = 2;
    float space = 2.0f;
    for (int i = 0;i < size*size*size;++i) {
        int zInt = i / (size * size);
        int yInt = (i % (size * size)) / size;
        int xInt = (i % (size * size)) % size;
        float z = space * (float)zInt - space * (float)(size - 1) / 2.0f;
        float y = space * (float)yInt - space * (float)(size - 1) / 2.0f;
        float x = space * (float)xInt - space * (float)(size - 1) / 2.0f;
        Object o = Object::temp({x, y, z});
        o.setTexture(mTextureManager.getTexture("dirt"));
        mObjects.push_back(std::make_unique<Object>(std::move(o)));
    }
    for (size_t i{0};i < mObjects.size();++i) {
        mObjectManager.addObject(*mObjects[i]);
        mObjectManager.updateBuffers();
    }

    mMeshManager.create(mContext);
    Vertex v{};
    mTemp = std::make_unique<Mesh>(std::vector<Vertex>({v, v, v}), std::vector<uint32_t>({1, 2, 3}));
    mTemp->setTexture(mTextureManager.getTexture("diamond"));
    mMeshManager.addMesh(*mTemp);

    mRenderer.create(mContext, mTextureManager, mObjectManager);
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
