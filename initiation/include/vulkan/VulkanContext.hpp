#ifndef VULKANCONTEXT
#define VULKANCONTEXT

#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "vulkan/QueueFamilyIndices.hpp"
#include "vulkan/CommandPool.hpp"
#include "memory/MemoryManager.hpp"

class VulkanContext {
    public:
        VulkanContext();
        void create(GLFWwindow* window);
        void destroy();

        VkInstance getInstance() const;
        VkPhysicalDevice getPhysicalDevice() const;
        VkDevice getDevice() const;
        VkQueue getGraphicsQueue() const;
        VkQueue getPresentQueue() const;
        VkQueue getTransferQueue() const;
        QueueFamilyIndices getQueueFamilyIndices() const;
        CommandPool& getGraphicsCommandPool();
        CommandPool& getTransferCommandPool();
        VkSurfaceKHR getSurface() const;
        GLFWwindow* getWindow() const;
        MemoryManager& getMemoryManager();

    private:
        GLFWwindow* mWindow;
        VkInstance mInstance;                               // Vulkan instance
        VkPhysicalDevice mPhysicalDevice{VK_NULL_HANDLE};   // Vulkan physical device handler
        VkDevice mDevice;                                   // Vulkan logical device handler
        VkQueue mGraphicsQueue;                             // Device graphic queue
        VkQueue mPresentQueue;                              // Device present queue
        VkQueue mTransferQueue;                              // Device present queue
        VkSurfaceKHR mSurface;                              // Vulkan surface handler
        QueueFamilyIndices mIndices;
        CommandPool mGraphicsCommandPool;
        CommandPool mTransferCommandPool;
        VkDebugUtilsMessengerEXT mCallback;                 // Message callback for validation layer
        MemoryManager mMemoryManager;

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
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void setupDebugCallback();
    
        bool checkValidationLayerSupport();

        std::vector<const char*> getRequiredExtensions();

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);
};

#endif