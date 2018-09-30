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

VkResult createDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pCallback);
void destroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT pCallback,
    const VkAllocationCallbacks* pAllocator);

class HelloTriangleApplication {
    public:
        void run();

    private:
        GLFWwindow* mWindow;                            // Window handler
        VkInstance mInstance;                           // Vulkan instance

        VkDebugUtilsMessengerEXT mCallback;             // Message callback for validation layer

        #ifdef DEBUG
            const bool enableValidationLayers = true;   // We want the validation layer in debug mode
        #else
            const bool enableValidationLayers = false;  // We don't want the validation layer otherwise
        #endif

        // Wanted validation layers
        const std::vector<const char*> validationLayers = {
            "VK_LAYER_LUNARG_standard_validation"
        };

        // Initialize Window
        void initWindow();

        // Initialize Vulkan
        void initVulkan();

        // Create Vulkan instance
        void createInstance();

        // Setup the debug callback function
        void setupDebugCallback();

        // App main loop
        void mainLoop();

        // Cleanup function
        void cleanup();

        // Check that the wanted validation layers are available
        bool checkValidationLayerSupport();
        
        // Retrieve the required extensions
        std::vector<const char*> getRequiredExtensions();

        // Debug message callback
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);
};

#endif