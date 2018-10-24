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
#include "vulkan/Shader.hpp"
#include "vulkan/QueueFamilyIndices.hpp"
#include "vulkan/Image.hpp"
#include "vulkan/CommandPool.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/ColorAttachment.hpp"
#include "vulkan/DepthAttachment.hpp"
#include "vulkan/Subpass.hpp"
#include "vulkan/SubpassDependency.hpp"
#include "vulkan/GraphicsPipeline.hpp"
#include "vulkan/SwapChain.hpp"
#include "primitives/Cube.hpp"
#include "primitives/StaticObject.hpp"
#include "vulkan/StaticObjectsManager.hpp"

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
        VkDescriptorSetLayout mDescriptorSetLayout;
        VkDescriptorSetLayout mColorDescriptorSetLayout;
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
        Shader mFragmentColorShader;
        Shader mVertexShader;

        Cube cube{0.5f};

        StaticObject sObject{
            {
                {{1.0f, 1.0f, 1.0f}, {.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
                {{-1.0f, 1.0f, 1.0f}, {.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
                {{-1.0f, -1.0f, 1.0f}, {.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
                {{1.0f, -1.0f, 1.0f}, {.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
                {{1.0f, 1.0f, -1.0f}, {.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
                {{-1.0f, 1.0f, -1.0f}, {.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
                {{-1.0f, -1.0f, -1.0f}, {.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
                {{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}
            },
            {
                1, 2, 3, 1, 3, 4,
                5, 1, 4, 5, 4, 8,
                2, 6, 7, 2, 7, 3,
                5, 6, 2, 5, 2, 1,
                4, 3, 7, 4, 7, 8,
                6, 5, 8, 6, 8, 7
            }
        };
        StaticObjectsManager sObjectManager;
        VkDescriptorSet mSODescriptorSet;

        GraphicsPipeline mGraphicsPipeline;
        GraphicsPipeline mGraphicsPipeline2;
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
        void createSODescriptorSets();
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