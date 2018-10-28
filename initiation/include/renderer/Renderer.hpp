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
#include "memory/MemoryManager.hpp"

class Renderer {
    public:
        void create(VkInstance instance);
        void destroy();
        void render();

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

        GLFWwindow* mWindow;
        MemoryManager* mMemoryManager;

        SwapChain mSwapChain;
        CommandPool mCommandPool;
        RenderPass mRenderPass;
        VkImage mDepthImage;
        VkImageView mDepthImageView;
        StaticObjectsManager mStaticObjectManager;
        GraphicsPipeline mPipeline;

        Shader mVertexShader;
        Shader mFragmentShader;

        void createSurface();
        void createRenderPass();
        void createGraphicsPipeline();
        void createFrameBuffers();
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