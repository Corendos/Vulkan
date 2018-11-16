#ifndef SWAPCHAIN
#define SWAPCHAIN

#include <vector>
#include <limits>

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan/QueueFamilyIndices.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/ImageView.hpp"

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
                    VkImageView depthImageView,
                    RenderPass& renderPass);
        void destroy(VkDevice device);

        VkExtent2D getExtent() const;
        VkFormat getFormat() const;
        VkSwapchainKHR getHandler() const;
        uint32_t getImageCount() const;
        std::vector<VkFramebuffer>& getFramebuffers();

    private:
        VkSwapchainKHR mSwapChain;

        std::vector<VkImage> mImages;
        //std::vector<VkImageView> mImagesView;
        std::vector<ImageView> mImagesView;
        std::vector<VkFramebuffer> mSwapChainFrameBuffers;
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
        void createFrameBuffers(VkDevice device,
                                RenderPass& renderPass,
                                VkImageView depthImageView);

        SwapChainSupportDetails querySupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);
        VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
};

#endif