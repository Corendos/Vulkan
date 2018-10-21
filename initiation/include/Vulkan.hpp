#ifndef VULKAN
#define VULKAN

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <optional>
#include <set>
#include <fstream>

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "utils.hpp"
#include "memory/MemoryManager.hpp"
#include "environment.hpp"
#include "shaders/Shader.hpp"
#include "device/QueueFamilyIndices.hpp"
#include "device/Image.hpp"
#include "device/CommandPool.hpp"
#include "device/RenderPass.hpp"
#include "device/ColorAttachment.hpp"
#include "device/DepthAttachment.hpp"
#include "device/Subpass.hpp"
#include "SwapChain.hpp"

class Vulkan {
    public:
    Vulkan();

    void init(GLFWwindow* window, int width, int height);
    void cleanup();
    void drawFrame();
    void requestResize();

    VkDevice& getDevice();

    private:
        GLFWwindow* mWindow;
        VkInstance mInstance;                               // Vulkan instance
        VkPhysicalDevice mPhysicalDevice{VK_NULL_HANDLE};   // Vulkan physical device handler
        VkDevice mDevice;                                   // Vulkan logical device handler
        VkQueue mGraphicsQueue;                             // Device graphic queue
        VkQueue mPresentQueue;                              // Device present queue
        VkSurfaceKHR mSurface;                              // Vulkan surface handler
        VkPipelineLayout mPipelineLayout;                   // Vulkan pipeline layout handler
        VkDescriptorSetLayout mDescriptorSetLayout;
        VkPipeline mGraphicsPipeline;                       // Vulkan pipeline handler
        std::vector<VkFramebuffer> mSwapChainFrameBuffers;  // Vulkan framebuffers handlers
        std::vector<VkCommandBuffer> mCommandBuffers;       // Vulkan command buffers
        VkSemaphore mImageAvailableSemaphore;               // Semaphore handling image availability
        VkSemaphore mRenderFinishedSemaphore;               // Semaphore handling rendering
        QueueFamilyIndices mIndices;
        VkDebugUtilsMessengerEXT mCallback;                 // Message callback for validation layer
        VkBuffer mVertexBuffer;
        VkDeviceMemory mVertexBufferMemory;
        VkBuffer mIndicesBuffer;
        VkDeviceMemory mIndicesBufferMemory;
        std::vector<VkBuffer> mUniformBuffers;
        std::vector<VkDeviceMemory> mUniformBuffersMemory;
        VkDescriptorPool mDescriptorPool;
        std::vector<VkDescriptorSet> mDescriptorSets;
        VkImage mTextureImage;
        VkImageView mTextureImageView;
        VkSampler mTextureSampler;

        SwapChain mSwapChain;
        CommandPool mCommandPool;
        RenderPass mRenderPass;

        VkImage mDepthImage;
        VkImageView mDepthImageView;

        Shader mFragmentShader;
        Shader mVertexShader;

        MemoryManager mMemoryManager;
        
        VkRect2D mWindowSize;
        bool mResizeRequested{false};

        const std::string shaderPath = std::string(ROOT_PATH) + std::string("shaders/build/");
        const std::vector<const char*> validationLayers = {
            "VK_LAYER_LUNARG_standard_validation"
        };
        const std::vector<const char*> deviceExtension = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        #ifdef DEBUG
            const bool enableValidationLayers{true};       // We want the validation layer in debug mode
        #else
            const bool enableValidationLayers{false};      // We don't want the validation layer otherwise
        #endif

        void createInstance();
        void setupDebugCallback();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createImageViews();
        void createRenderPass();
        void createDescriptorSetLayout();
        void createGraphicsPipeline();
        void createFrameBuffers();
        void createDepthResources();
        void createTextureImage();
        void createTextureImageView();
        void createTextureSampler();
        void createVertexBuffer();
        void createIndicesBuffer();
        void createUniformBuffer();
        void createDescriptorPool();
        void createDescriptorSets();
        void createCommandBuffers();
        void createSemaphores();
        
        void updateUniformData(uint32_t imageIndex);

        void recreateSwapChain();
        void cleanupSwapChain();
        bool checkValidationLayerSupport();
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties, VkBuffer& buffer);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

        VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

        std::vector<const char*> getRequiredExtensions();
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        VkFormat findDepthFormat();
        bool hasStencilComponent(VkFormat format);

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);
};

#endif