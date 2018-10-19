#ifndef SWAPCHAIN
#define SWAPCHAIN

#include <vector>
#include <limits>

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "device/QueueFamilyIndices.hpp"

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentMode;
};

class SwapChain {
    public:
        SwapChain();

        void create(VkPhysicalDevice physicalDevice, VkDevice device, GLFWwindow* window, VkSurfaceKHR surface, QueueFamilyIndices indices);
        void create();
        void recreate();
        void destroy();

        VkExtent2D getExtent() const;
        std::vector<VkImageView>& getImagesView();
        VkFormat getFormat() const;
        VkSwapchainKHR getHandler() const;

    private:
        VkDevice mDevice;
        VkPhysicalDevice mPhysicalDevice;
        GLFWwindow* mWindow;
        VkSurfaceKHR mSurface;
        QueueFamilyIndices mQueueIndices;

        VkSwapchainKHR mSwapChain;

        std::vector<VkImage> mImages;
        std::vector<VkImageView> mImagesView;
        VkExtent2D mExtent;
        VkFormat mImageFormat;
        uint32_t mImageCount;

        SwapChainSupportDetails querySupport();
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
        VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
};

#endif