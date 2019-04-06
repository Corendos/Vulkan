#ifndef RENDERER
#define RENDERER

#include <vector>
#include <memory>
#include <future>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include "vulkan/SwapChain.hpp"
#include "vulkan/CommandPool.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/GraphicsPipeline.hpp"
#include "vulkan/Shader.hpp"
#include "vulkan/VulkanContext.hpp"
#include "renderer/camera/Camera.hpp"
#include "renderer/mesh/MeshManager.hpp"
#include "renderer/light/Light.hpp"
#include "memory/MemoryManager.hpp"
#include "resources/TextureManager.hpp"
#include "environment.hpp"

struct FrameBufferAttachment {
    vk::Image image;
    vk::ImageView imageView;
};

struct RendererAttachments {
    FrameBufferAttachment normal;
    FrameBufferAttachment depth;
};

struct FenceInfo {
    bool submitted{false};
    VkFence fence;
};

using AsyncUpdateResult = std::vector<VkCommandBuffer>;

enum CommandBufferState { NotReady, Ready, Size };

class Renderer {
    public:
        Renderer();
        void create(VulkanContext& context, TextureManager& textureManager, MeshManager& meshManager);
        void recreate();
        void destroy();
        void render();
        void update(double dt);

        void setCamera(Camera& camera);
        void setLight(Light& light);

        MemoryManager& getMemoryManager();
        VkDescriptorPool getDescriptorPool() const;
        SwapChain& getSwapChain();

    private:
        VkSemaphore mImageAvailableSemaphore;
        std::vector<VkSemaphore> mRenderFinishedSemaphores;
        VkDescriptorPool mDescriptorPool;
        VkDescriptorSetLayout mCameraDescriptorSetLayout;
        VkDescriptorSet mDescriptorSet;
        VkExtent2D mExtent;
        std::vector<FenceInfo> mFencesInfo;
        std::vector<VkCommandPool> mCommandPools;
        std::vector<VkCommandBuffer> mCommandBuffers;

        std::vector<Framebuffer> mFrameBuffers;

        Camera* mCamera;
        Light* mLight;
        VulkanContext* mContext;
        TextureManager* mTextureManager;

        SwapChain mSwapChain;
        RenderPass mRenderPass;
        GraphicsPipeline mPipeline;

        Shader mVertexShader;
        Shader mFragmentShader;

        MeshManager* mMeshManager;

        uint32_t mNextImageIndex;
        std::array<VkClearValue, 3> mClearValues;

        std::vector<vk::Buffer> mCameraUniformBuffers;
        std::vector<VkDescriptorSet> mCameraDescriptorSets;

        std::vector<RendererAttachments> mFramebufferAttachments;

        std::vector<VkSemaphore> mToWaitSemaphores;

        bool mCreated{false};
        bool mBypassRendering{false};

        const std::string shaderPath = std::string(ROOT_PATH) + std::string("resources/shaders/build/");

        void acquireNextImage();
        void waitForFence();

        void createRenderPass();
        void createGraphicsPipeline();
        void createFramebuffers();
        void createDescriptorPool();
        void createDescriptorSetLayout();
        void createCommandBuffers();
        void createCameraUniformBuffers();
        void createCameraDescriptorSets();
        void createSemaphores();
        void createFences();
        void createCommandPools();

        FrameBufferAttachment createAttachment(VkFormat format,
                                               VkImageUsageFlags usage,
                                               VkImageAspectFlags aspect);
        void updateUniformBuffer(uint32_t index);

        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        VkFormat findDepthFormat();
};

#endif