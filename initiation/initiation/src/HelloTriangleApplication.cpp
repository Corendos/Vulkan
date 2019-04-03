#include <thread>
#include <iomanip>

#include "HelloTriangleApplication.hpp"
#include "utils.hpp"
#include "renderer/mesh/MeshHelper.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

void HelloTriangleApplication::run() {
    init();
    mainLoop();
    cleanup();
}

void HelloTriangleApplication::mainLoop() {
    int i{0};
    uint32_t updateMean{0}, renderMean{0};
    double frameMean{0.0};
    mFrameStartTime = std::chrono::high_resolution_clock::now();

    while(!glfwWindowShouldClose(mWindow)) {
        mLastFrameStartTime = mFrameStartTime;
        mFrameStartTime = std::chrono::high_resolution_clock::now();
        glfwPollEvents();

        processInputs();

        double dt = std::chrono::duration<double>(mFrameStartTime - mLastFrameStartTime).count();
        if (dt > TARGET_FRAME_TIME) {
            epsilonPadding += std::chrono::duration<double>(dt - TARGET_FRAME_TIME) ;
        } else if (dt < TARGET_FRAME_TIME) {
            epsilonPadding += std::chrono::duration<double>(dt - TARGET_FRAME_TIME);
        }

        mDeer.getTransform().rotate(glm::half_pi<double>() * dt, glm::vec3(0.0, 0.0, 1.0));

        auto start = std::chrono::high_resolution_clock::now();
        mRenderer.update(dt);
        auto updateEnd = std::chrono::high_resolution_clock::now();
        mRenderer.render();
        auto end = std::chrono::high_resolution_clock::now();
        uint32_t updateDuration = std::chrono::duration_cast<std::chrono::microseconds>(updateEnd - start).count();
        uint32_t renderDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - updateEnd).count();
        
        updateMean += updateDuration;
        renderMean += renderDuration;
        frameMean += dt;
        i++;

        if (i % 10 == 0) {
            std::cout << "Update: " << (float)updateMean / 10.0f << "µs" << std::endl;
            std::cout << "Render: " << (float)renderMean / 10.0f << "µs" << std::endl;
            std::cout << std::fixed << std::setprecision(5) << "Frame: " << 10.0 / frameMean << "fps"
                << std::defaultfloat << std::endl;
            i = 0;
            updateMean = 0;
            renderMean = 0;
            frameMean = 0.0;
        }
        
        sleepUntilNextFrame();
    }
}

void HelloTriangleApplication::cleanup() {
    vkDeviceWaitIdle(mContext.getDevice());
    mInput.stop();
    mMeshManager.destroy();
    mRenderer.destroy();
    mTextureManager.destroy();
    mContext.destroy();
    glfwDestroyWindow(mWindow);
    glfwTerminate();
    mFileWatch.stop();
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

    mLight = {{20.0, 20.0, 20.0}};

    mTextureManager.create(mContext);
    mTextureManager.load("diamond", std::string(ROOT_PATH) + std::string("resources/textures/diamond.png"));
    mTextureManager.load("dirt", std::string(ROOT_PATH) + std::string("resources/textures/dirt.png"));
    mTextureManager.load("undefined", std::string(ROOT_PATH) + std::string("resources/textures/undefined.png"));
    mTextureManager.load("cottage_diffuse", std::string(ROOT_PATH) + std::string("resources/textures/cottage_diffuse.png"));

    mMeshManager.create(mContext);

    mRenderer.create(mContext, mTextureManager, mMeshManager);
    mRenderer.setCamera(mCamera);
    mRenderer.setLight(mLight);

    mMeshManager.setImageCount(mRenderer.getSwapChain().getImageCount());

    mDeer = mImporter.loadMesh("cottage.fbx");
    mDeer.setTexture(mTextureManager.getTexture("cottage_diffuse"));
    mDeer.getTransform().setScale({3.0, 1.0, 1.0});
    mMeshManager.addMesh(mDeer);

    mTemp = std::make_unique<Mesh>(std::move(MeshHelper::createCube(1.0)));
    mTemp->setTexture(mTextureManager.getTexture("diamond"));
    mTemp->getTransform().setPosition({-2.0, -2.0, -2.0});
    mMeshManager.addMesh(*mTemp);

    mFileWatch.launch();
    mFileWatch.watchFile("/home/corentin/", "test", [](){ std::cout << "callback" << std::endl; });
}

void HelloTriangleApplication::sleepUntilNextFrame() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration<double>(currentTime - mFrameStartTime);
    auto target = std::chrono::duration<double>(1.0 / TARGET_FPS);
    
    if (elapsed < target) {
        std::this_thread::sleep_for(target - elapsed - epsilonPadding);
    }
}

void HelloTriangleApplication::processInputs() {
    if (mInput.getMouse().button[MouseButton::Left].pressed) {
        double deltaYaw = mInput.getMouse().delta.x * -0.5;
        double deltaPitch = mInput.getMouse().delta.y * 0.5;

        mCamera.update(deltaPitch, deltaYaw);
    }

    if (glfwGetKey(mWindow, GLFW_KEY_SPACE) && !mTempKeyState) {
        mMeshes.push_back(std::make_unique<Mesh>(std::move(MeshHelper::createCube(1.0))));

        Mesh* temp = mMeshes.back().get();
        temp->setTexture(mTextureManager.getTexture("diamond"));
        temp->getTransform().setPosition({mTempCounter, mTempCounter, mTempCounter});
        mMeshManager.addMesh(*temp);
        mTempCounter += 1.0f;
        mTempKeyState = true;
    } else if (!glfwGetKey(mWindow, GLFW_KEY_SPACE)){
        mTempKeyState = false;
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
