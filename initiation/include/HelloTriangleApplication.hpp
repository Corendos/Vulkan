#ifndef HELLOTRIANGLEAPPLICATION
#define HELLOTRIANGLEAPPLICATION

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <optional>
#include <set>
#include <fstream>

VkResult createDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pCallback);
void destroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT pCallback,
    const VkAllocationCallbacks* pAllocator);

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentMode;
};

class HelloTriangleApplication {
    public:
        void run();

    private:
        GLFWwindow* mWindow;                                // Window handler
        VkInstance mInstance;                               // Vulkan instance
        VkPhysicalDevice mPhysicalDevice{VK_NULL_HANDLE};   // Vulkan physical device handler
        VkDevice mDevice;                                   // Vulkan logical device handler
        VkQueue mGraphicsQueue;                             // Device graphic queue
        VkQueue mPresentQueue;                              // Device present queue
        VkSurfaceKHR mSurface;                              // Vulkan surface handler 
        VkSwapchainKHR mSwapChain;                          // Vulkan swap chain handler
        std::vector<VkImage> mSwapChainImages;              // Vulkan swap chain images
        VkFormat mSwapChainImageFormat;                     // Vulkan swap chain image format
        VkExtent2D mSwapChainExtent;                        // Vulkan swap chain extent
        std::vector<VkImageView> mSwapChainImageViews;      // Vulkan swap chain image views
        VkRenderPass mRenderPass;                           // Vulkan render pass handler
        VkPipelineLayout mPipelineLayout;                   // Vulkan pipeline layout handler
        VkPipeline mGraphicsPipeline;                       // Vulkan pipeline handler
        std::vector<VkFramebuffer> mSwapChainFrameBuffers;  // Vulkan framebuffers handlers
        VkCommandPool mCommandPool;                         // Vulkan command pool
        std::vector<VkCommandBuffer> mCommandBuffers;       // Vulkan command buffers

        VkDebugUtilsMessengerEXT mCallback;                 // Message callback for validation layer

        #ifdef DEBUG
            const bool enableValidationLayers{true};       // We want the validation layer in debug mode
        #else
            const bool enableValidationLayers{false};      // We don't want the validation layer otherwise
        #endif

        const unsigned int WIDTH{800};
        const unsigned int HEIGHT{600};

        // Wanted validation layers
        const std::vector<const char*> validationLayers = {
            "VK_LAYER_LUNARG_standard_validation"
        };

        // Required device extensions
        const std::vector<const char*> deviceExtension = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        // Initialize Window
        void initWindow();

        // Initialize Vulkan
        void initVulkan();

        // Create Vulkan instance
        void createInstance();

        // Setup the debug callback function
        void setupDebugCallback();

        // Pick the physical device to use()
        void pickPhysicalDevice();

        // Check if the device is suitable for our use
        bool isDeviceSuitable(VkPhysicalDevice device);

        // Check that the device support the required extension
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);

        // Retrieve the device's queue families
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

        // Get the swap chain support
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

        // Choose the best swap surface format
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

        // Choose the best swap present mode
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

        // Choose the swap extent
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        // Create the graphics pipeline
        void createGraphicsPipeline();

        // Create the render pass
        void createRenderPass();

        // Create the frame buffers
        void createFrameBuffers();

        // Create the command pool
        void createCommandPool();

        // Create the command buffers
        void createCommandBuffers();

        // Create the image views to use the images as color targets
        void createImageViews();

        // Create the logical device
        void createLogicalDevice();

        // Create the swap chain
        void createSwapChain();

        // Create the window surface
        void createSurface();

        // App main loop
        void mainLoop();

        // Cleanup function
        void cleanup();

        // Check that the wanted validation layers are available
        bool checkValidationLayerSupport();

        // Create a shader module from the given code
        VkShaderModule createShaderModule(const std::vector<char>& code);
        
        // Retrieve the required extensions
        std::vector<const char*> getRequiredExtensions();

        // Debug message callback
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);
        
        static std::vector<char> readFile(const std::string& filename);
};

#endif