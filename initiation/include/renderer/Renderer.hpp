#ifndef RENDERER
#define RENDERER

#include <vector>
#include <memory>

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
#include "renderer/ObjectManager.hpp"
#include "renderer/Light.hpp"
#include "memory/MemoryManager.hpp"
#include "renderer/Object.hpp"
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
        void setLight(Light& light);

        MemoryManager& getMemoryManager();
        VkDescriptorPool getDescriptorPool() const;
        VkDescriptorSetLayout getDescriptorSetLayout() const;

    private:
        VkSemaphore mImageAvailableSemaphore;
        VkSemaphore mRenderFinishedSemaphore;
        VkDescriptorPool mDescriptorPool;
        VkDescriptorSetLayout mTextureDescriptorSetLayout;
        VkDescriptorSetLayout mCameraDescriptorSetLayout;
        VkDescriptorSet mDescriptorSet;
        VkExtent2D mExtent;
        std::vector<VkCommandBuffer> mCommandBuffers;
        std::vector<VkFence> mFences;
        std::vector<bool> mIsFenceSubmitted;
        std::vector<bool> mCommandBufferNeedUpdate;

        Camera* mCamera;
        Light* mLight;
        VulkanContext* mContext;

        SwapChain mSwapChain;
        RenderPass mRenderPass;
        VkImage mDepthImage;
        VkImageView mDepthImageView;
        GraphicsPipeline mPipeline;

        Shader mVertexShader;
        Shader mFragmentShader;

        ObjectManager mObjectManager;
        std::vector<std::unique_ptr<Object>> mObjects;

        uint32_t mNextImageIndex;
        std::array<VkClearValue, 2> mClearValues;

        std::vector<VkBuffer> mCameraUniformBuffers;
        std::vector<VkDescriptorSet> mCameraDescriptorSets;

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
        void createCameraUniformBuffers();
        void createCameraDescriptorSets();
        void createSemaphores();
        void createFences();

        void updateCommandBuffer(uint32_t index);
        void updateUniformBuffer(uint32_t index);

        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        VkFormat findDepthFormat();
};

#endif