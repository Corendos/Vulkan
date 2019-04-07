#ifndef RENDERER
#define RENDERER

#include <vector>
#include <memory>
#include <future>

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include "vulkan/SwapChain.hpp"
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
    vk::Fence fence;
};

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
        vk::DescriptorPool getDescriptorPool() const;
        SwapChain& getSwapChain();

    private:
        vk::Semaphore mImageAvailableSemaphore;
        std::vector<vk::Semaphore> mRenderFinishedSemaphores;
        vk::DescriptorPool mDescriptorPool;
        vk::DescriptorSetLayout mCameraDescriptorSetLayout;
        vk::DescriptorSet mDescriptorSet;
        vk::Extent2D mExtent;
        std::vector<FenceInfo> mFencesInfo;
        std::vector<vk::CommandPool> mCommandPools;
        std::vector<vk::CommandBuffer> mCommandBuffers;

        std::vector<vk::Framebuffer> mFrameBuffers;

        Camera* mCamera;
        Light* mLight;
        VulkanContext* mContext;
        TextureManager* mTextureManager;

        SwapChain mSwapChain;
        vk::RenderPass mRenderPass;
        vk::Pipeline mPipeline;
        vk::PipelineLayout mPipelineLayout;

        Shader::Pair mVertexShader;
        Shader::Pair mFragmentShader;

        MeshManager* mMeshManager;

        uint32_t mNextImageIndex;
        std::array<vk::ClearValue, 3> mClearValues;

        std::vector<vk::Buffer> mCameraUniformBuffers;
        std::vector<vk::DescriptorSet> mCameraDescriptorSets;

        std::vector<RendererAttachments> mFramebufferAttachments;

        std::vector<vk::Semaphore> mToWaitSemaphores;

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

        FrameBufferAttachment createAttachment(vk::Format format,
                                               vk::ImageUsageFlags usage,
                                               vk::ImageAspectFlags aspect);
        void updateUniformBuffer(uint32_t index);

        vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates,
                                       vk::ImageTiling tiling,
                                       vk::FormatFeatureFlags features);
        vk::Format findDepthFormat();
};

#endif