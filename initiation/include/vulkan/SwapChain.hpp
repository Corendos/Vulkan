#ifndef SWAPCHAIN
#define SWAPCHAIN

#include <vector>
#include <limits>

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan/QueueFamilyIndices.hpp"

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentMode;
};

class SwapChain {
    public:
        SwapChain();

        void create(VkPhysicalDevice physicalDevice, VkDevice device, GLFWwindow* window, VkSurfaceKHR surface, QueueFamilyIndices indices);
        void destroy(VkDevice device);

        VkExtent2D getExtent() const;
        VkFormat getFormat() const;
        VkSwapchainKHR getHandler() const;
        uint32_t getImageCount() const;
        std::vector<VkImageView>& getImagesView();

    private:
        VkSwapchainKHR mSwapChain;

        std::vector<VkImage> mImages;
        std::vector<VkImageView> mImagesView;
        VkExtent2D mExtent;
        VkFormat mImageFormat;
        uint32_t mImageCount;

        bool mCreated{false};

        SwapChainSupportDetails querySupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);
        VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
};

#endif