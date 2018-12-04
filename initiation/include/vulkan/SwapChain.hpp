#ifndef SWAPCHAIN
#define SWAPCHAIN

#include <vector>
#include <limits>

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan/QueueFamilyIndices.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/image/ImageView.hpp"
#include "vulkan/Framebuffer.hpp"

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentMode;
};

class SwapChain {
    public:
        SwapChain();

        void query(GLFWwindow* window,
                   VkPhysicalDevice physicalDevice,
                   VkDevice device,
                   VkSurfaceKHR surface);
        void create(GLFWwindow* window,
                    VkPhysicalDevice physicalDevice,
                    VkDevice device,
                    VkSurfaceKHR surface,
                    QueueFamilyIndices indices,
                    RenderPass& renderPass);
        void destroy(VkDevice device);

        VkExtent2D getExtent() const;
        VkFormat getFormat() const;
        VkSwapchainKHR getHandler() const;
        uint32_t getImageCount() const;
        VkImageView getImageView(uint32_t index);

    private:
        VkSwapchainKHR mSwapChain;

        std::vector<VkImage> mImages;
        std::vector<ImageView> mImagesView;
        VkExtent2D mExtent;
        VkSurfaceFormatKHR mSurfaceFormat;
        SwapChainSupportDetails mSwapChainSupport;
        VkPresentModeKHR mPresentMode;
        uint32_t mImageCount;

        bool mCreated{false};

        void createSwapChain(GLFWwindow* window,
                             VkPhysicalDevice physicalDevice,
                             VkDevice device,
                             VkSurfaceKHR surface,
                             QueueFamilyIndices indices);
        void createImages(VkDevice device);
        void createImageViews(VkDevice device);

        SwapChainSupportDetails querySupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);
        VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
};

#endif