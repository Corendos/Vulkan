#ifndef VULKANCONTEXT
#define VULKANCONTEXT

#include <vector>
#include <thread>
#include <unordered_map>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include "vulkan/QueueFamilyIndices.hpp"
#include "memory/MemoryManager.hpp"

class VulkanContext {
    public:
        VulkanContext();
        void create(GLFWwindow* window);
        void destroy();

        vk::Instance getInstance() const;
        vk::PhysicalDevice getPhysicalDevice() const;
        vk::Device getDevice() const;
        vk::PhysicalDeviceLimits getLimits() const;
        vk::Queue getPresentQueue() const;
        vk::Queue getTransferQueue() const;
        vk::Queue getGraphicsQueue() const;
        QueueFamilyIndices getQueueFamilyIndices() const;
        vk::CommandPool& getTransferCommandPool();
        vk::SurfaceKHR getSurface() const;
        GLFWwindow* getWindow() const;
        MemoryManager& getMemoryManager();
        vk::DescriptorPool& getDescriptorPool();

    private:
        GLFWwindow* mWindow;
        vk::Instance mInstance;                               // Vulkan instance
        vk::PhysicalDevice mPhysicalDevice;   // Vulkan physical device handler
        vk::Device mDevice;                                   // Vulkan logical device handler
        vk::Queue mPresentQueue;                              // Device present queue
        vk::Queue mTransferQueue;                             // Device transfer queue
        vk::Queue mGraphicsQueue;                             // Device graphics queue
        vk::SurfaceKHR mSurface;                              // Vulkan surface handler
        QueueFamilyIndices mIndices;
        std::unordered_map<std::thread::id, vk::CommandPool> mTransferCommandPoolMap;
        VkDebugUtilsMessengerEXT mCallback;                 // Message callback for validation layer
        MemoryManager mMemoryManager;
        vk::PhysicalDeviceLimits mPhysicalDeviceLimits;
        vk::DescriptorPool mDescriptorPool;

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
        void createDescriptorPool();
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