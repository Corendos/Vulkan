#ifndef SWAPCHAIN
#define SWAPCHAIN

#include <vector>
#include <limits>

#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan/QueueFamilyIndices.hpp"

struct SwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentMode;
};

class SwapChain {
    public:
        SwapChain();

        void query(GLFWwindow* window,
                   vk::PhysicalDevice physicalDevice,
                   vk::Device device,
                   vk::SurfaceKHR surface);
        void create(GLFWwindow* window,
                    vk::PhysicalDevice physicalDevice,
                    vk::Device device,
                    vk::SurfaceKHR surface,
                    QueueFamilyIndices indices,
                    vk::RenderPass& renderPass);
        void destroy(vk::Device device);

        vk::Extent2D getExtent() const;
        vk::Format getFormat() const;
        vk::SwapchainKHR getHandler() const;
        uint32_t getImageCount() const;
        vk::ImageView getImageView(uint32_t index);

    private:
        vk::SwapchainKHR mSwapChain;

        std::vector<vk::Image> mImages;
        std::vector<vk::ImageView> mImagesView;
        vk::Extent2D mExtent;
        vk::SurfaceFormatKHR mSurfaceFormat;
        SwapChainSupportDetails mSwapChainSupport;
        vk::PresentModeKHR mPresentMode;
        uint32_t mImageCount;

        bool mCreated{false};

        void createSwapChain(GLFWwindow* window,
                             vk::PhysicalDevice physicalDevice,
                             vk::Device device,
                             vk::SurfaceKHR surface,
                             QueueFamilyIndices indices);
        void createImages(vk::Device device);
        void createImageViews(vk::Device device);

        SwapChainSupportDetails querySupport(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);
        vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
        vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
        vk::Extent2D chooseSwapExtent(GLFWwindow* window, const vk::SurfaceCapabilitiesKHR& capabilities);
};

#endif