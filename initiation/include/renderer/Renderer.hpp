#ifndef RENDERER
#define RENDERER

#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "vulkan/StaticObjectsManager.hpp"
#include "vulkan/SwapChain.hpp"
#include "vulkan/CommandPool.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/GraphicsPipeline.hpp"
#include "vulkan/Shader.hpp"
#include "camera/Camera.hpp"
#include "memory/MemoryManager.hpp"
#include "environment.hpp"

class Renderer {
    public:
        Renderer();
        void create(VkInstance instance,
                    GLFWwindow* window,
                    VkPhysicalDevice physicalDevice,
                    VkDevice device,
                    VkSurfaceKHR surface,
                    QueueFamilyIndices indices,
                    VkQueue graphicsQueue,
                    VkQueue presentQueue,
                    MemoryManager& memoryManager);
        void recreate();
        void destroy();
        void render();

        void setCamera(Camera& camera);

        MemoryManager& getMemoryManager();
        VkDevice getDevice() const;
        VkQueue getGraphicsQueue() const;
        CommandPool& getCommandPool();

        StaticObjectsManager& getStaticObjectManager();

    private:
        VkInstance mInstance;
        VkDevice mDevice;
        VkPhysicalDevice mPhysicalDevice;
        VkQueue mGraphicsQueue;
        VkQueue mPresentQueue;
        QueueFamilyIndices mIndices;
        VkSurfaceKHR mSurface;
        VkSemaphore mImageAvailableSemaphore;
        VkSemaphore mRenderFinishedSemaphore;
        VkDescriptorPool mDescriptorPool;
        VkDescriptorSetLayout mDescriptorSetLayout;
        VkDescriptorSet mDescriptorSet;
        VkExtent2D mExtent;
        std::vector<VkCommandBuffer> mCommandBuffers;

        GLFWwindow* mWindow;
        MemoryManager* mMemoryManager;
        Camera* mCamera;

        SwapChain mSwapChain;
        CommandPool mCommandPool;
        RenderPass mRenderPass;
        VkImage mDepthImage;
        VkImageView mDepthImageView;
        StaticObjectsManager mStaticObjectManager;
        GraphicsPipeline mPipeline;

        Shader mVertexShader;
        Shader mFragmentShader;

        bool mCreated{false};
        bool mRecreated{false};

        const std::string shaderPath = std::string(ROOT_PATH) + std::string("shaders/build/");

        void createRenderPass();
        void createGraphicsPipeline();
        void createDepthResources();
        void createDescriptorPool();
        void createDescriptorSetLayout();
        void createDescriptorSets();
        void createCommandBuffers();
        void createSemaphores();

        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        VkFormat findDepthFormat();
};

#endif