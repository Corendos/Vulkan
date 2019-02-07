#ifndef RENDERER
#define RENDERER

#include <vector>
#include <memory>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "vulkan/SwapChain.hpp"
#include "vulkan/CommandPool.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/GraphicsPipeline.hpp"
#include "vulkan/Shader.hpp"
#include "vulkan/image/Image.hpp"
#include "vulkan/VulkanContext.hpp"
#include "renderer/camera/Camera.hpp"
#include "renderer/mesh/MeshManager.hpp"
#include "renderer/light/Light.hpp"
#include "memory/MemoryManager.hpp"
#include "resources/TextureManager.hpp"
#include "environment.hpp"

struct FrameBufferAttachment {
    Image image;
    ImageView imageView;
};

struct RendererAttachments {
    FrameBufferAttachment normal;
    FrameBufferAttachment depth;
};

struct FenceInfo {
    bool submitted{false};
    VkFence fence;
};

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
        VkSemaphore mRenderFinishedSemaphore;
        VkDescriptorPool mDescriptorPool;
        VkDescriptorSetLayout mCameraDescriptorSetLayout;
        VkDescriptorSet mDescriptorSet;
        VkExtent2D mExtent;
        std::vector<VkCommandBuffer> mCommandBuffers;
        std::vector<FenceInfo> mFencesInfo;
        std::vector<bool> mCommandBufferNeedUpdate;

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

        std::vector<VkBuffer> mCameraUniformBuffers;
        std::vector<VkDescriptorSet> mCameraDescriptorSets;

        std::vector<RendererAttachments> mFramebufferAttachments;

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

        FrameBufferAttachment createAttachment(VkFormat format,
                                               VkImageUsageFlags usage,
                                               VkImageAspectFlags aspect);

        void updateCommandBuffer(uint32_t index);
        void updateUniformBuffer(uint32_t index);

        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        VkFormat findDepthFormat();
};

#endif