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
#include "vulkan/Image.hpp"
#include "vulkan/VulkanContext.hpp"
#include "camera/Camera.hpp"
#include "memory/MemoryManager.hpp"
#include "environment.hpp"

class Renderer {
    public:
        Renderer();
        void create(VulkanContext& context);
        void recreate();
        void destroy();
        void render();
        void update();

        void setCamera(Camera& camera);

        MemoryManager& getMemoryManager();
        CommandPool& getCommandPool();
        VkDescriptorPool getDescriptorPool() const;
        VkDescriptorSetLayout getDescriptorSetLayout() const;

        StaticObjectsManager& getStaticObjectManager();

    private:
        VkSemaphore mImageAvailableSemaphore;
        VkSemaphore mRenderFinishedSemaphore;
        VkDescriptorPool mDescriptorPool;
        VkDescriptorSetLayout mDescriptorSetLayout;
        VkDescriptorSet mDescriptorSet;
        VkExtent2D mExtent;
        std::vector<VkCommandBuffer> mCommandBuffers;
        std::vector<VkFence> mFences;
        std::vector<bool> mIsFenceSubmitted;

        Camera* mCamera;
        VulkanContext* mContext;

        SwapChain mSwapChain;
        CommandPool mCommandPool;
        RenderPass mRenderPass;
        VkImage mDepthImage;
        VkImageView mDepthImageView;
        StaticObjectsManager mStaticObjectManager;
        GraphicsPipeline mPipeline;

        Shader mVertexShader;
        Shader mFragmentShader;

        uint32_t mNextImageIndex;
        std::array<VkClearValue, 2> mClearValues;

        bool mCreated{false};
        bool mRecreated{false};
        bool mBypassRendering{false};

        const std::string shaderPath = std::string(ROOT_PATH) + std::string("shaders/build/");

        void createRenderPass();
        void createGraphicsPipeline();
        void createDepthResources();
        void createDescriptorPool();
        void createDescriptorSetLayout();
        void createCommandBuffers();
        void createSemaphores();
        void createFences();

        void updateCommandBuffer(uint32_t index);

        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        VkFormat findDepthFormat();
};

#endif