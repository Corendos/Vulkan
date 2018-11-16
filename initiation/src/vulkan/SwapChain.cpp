#include "vulkan/SwapChain.hpp"

#include <array>

SwapChain::SwapChain() {}

void SwapChain::query(GLFWwindow* window,
                      VkPhysicalDevice physicalDevice,
                      VkDevice device,
                      VkSurfaceKHR surface) {
    mSwapChainSupport = querySupport(physicalDevice, surface);

    mSurfaceFormat = chooseSwapSurfaceFormat(mSwapChainSupport.formats);
    mPresentMode = chooseSwapPresentMode(mSwapChainSupport.presentMode);
    mExtent = chooseSwapExtent(window, mSwapChainSupport.capabilities);

    mImageCount = mSwapChainSupport.capabilities.minImageCount + 1;
    if (mSwapChainSupport.capabilities.maxImageCount > 0 && mImageCount > mSwapChainSupport.capabilities.maxImageCount) {
        mImageCount = mSwapChainSupport.capabilities.maxImageCount;
    }
}

void SwapChain::create(GLFWwindow* window,
                       VkPhysicalDevice physicalDevice,
                       VkDevice device,
                       VkSurfaceKHR surface,
                       QueueFamilyIndices indices,
                       VkImageView depthImageView,
                       RenderPass& renderPass) {
    if (mCreated) {
        return;
    }

    createSwapChain(window, physicalDevice, device, surface, indices);
    createImages(device);
    createImageViews(device);
    createFrameBuffers(device, renderPass, depthImageView);

    mCreated = true;
}

void SwapChain::destroy(VkDevice device) {
    if (mCreated) {
        for (auto imageView : mImagesView) {
            imageView.destroy(device);
        }

        for (auto framebuffer : mSwapChainFrameBuffers) {
            framebuffer.destroy(device);
        }

        vkDestroySwapchainKHR(device, mSwapChain, nullptr);
        mCreated = false;
    }
}

VkExtent2D SwapChain::getExtent() const {
    return mExtent;
}

VkFramebuffer SwapChain::getFramebuffer(uint32_t index) {
    return mSwapChainFrameBuffers[index].getHandler();
}

VkFormat SwapChain::getFormat() const {
    return mSurfaceFormat.format;
}

VkSwapchainKHR SwapChain::getHandler() const {
    return mSwapChain;
}

uint32_t SwapChain::getImageCount() const {
    return mImageCount;
}

void SwapChain::createSwapChain(GLFWwindow* window,
                                VkPhysicalDevice physicalDevice,
                                VkDevice device,
                                VkSurfaceKHR surface,
                                QueueFamilyIndices indices) {
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = mImageCount;
    createInfo.imageFormat = mSurfaceFormat.format;
    createInfo.imageColorSpace = mSurfaceFormat.colorSpace;
    createInfo.imageExtent = mExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] = {
        indices.graphicsFamily.value(),
        indices.presentFamily.value()
    };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else  {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = mSwapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = mPresentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &mSwapChain) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain");
    }
}

void SwapChain::createImages(VkDevice device) {
    vkGetSwapchainImagesKHR(device, mSwapChain, &mImageCount, nullptr);
    mImages.resize(mImageCount);
    vkGetSwapchainImagesKHR(device, mSwapChain, &mImageCount, mImages.data());
}

void SwapChain::createImageViews(VkDevice device) {
    mImagesView.resize(mImageCount);

    for (size_t i{0}; i < mImages.size();++i) {
        mImagesView[i].setImage(mImages[i]);
        mImagesView[i].setImageViewType(VK_IMAGE_VIEW_TYPE_2D);
        mImagesView[i].setFormat(mSurfaceFormat.format);
        VkImageSubresourceRange range;
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseMipLevel = 0;
        range.levelCount = 1;
        range.baseArrayLayer = 0;
        range.layerCount = 1;
        mImagesView[i].setSubresourceRange(range);
        mImagesView[i].create(device);
    }
}

void SwapChain::createFrameBuffers(VkDevice device,
                                   RenderPass& renderPass,
                                   VkImageView depthImageView) {
    mSwapChainFrameBuffers.resize(mImageCount);

    for (size_t i{0}; i < mImageCount;++i) {
        std::vector<VkImageView> attachments = {
            mImagesView[i].getHandler(),
            depthImageView
        };
        mSwapChainFrameBuffers[i].setRenderPass(renderPass.getHandler());
        mSwapChainFrameBuffers[i].setAttachments(attachments);
        mSwapChainFrameBuffers[i].setWidth(mExtent.width);
        mSwapChainFrameBuffers[i].setHeight(mExtent.height);
        mSwapChainFrameBuffers[i].setLayers(1);
        mSwapChainFrameBuffers[i].create(device);
    }
}

SwapChainSupportDetails SwapChain::querySupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    /* Get surface capabilities */
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);
    
    /* Get surface supported formats */
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
    }

    /* Get surface supported present modes */
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentMode.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentMode.data());
    }

    return details;
}

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    /* If the only supported format is undefined, force the format we want */
    if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
        return {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    /* Otherwise, try to find the format we want in the list */
    for (const auto& format : availableFormats) {
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }

    /* If nothing was foudn return the first format of the list */
    return availableFormats[0];
}

VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    /* Find the best present mode */
    VkPresentModeKHR bestMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    
    for (const auto& presentMode : availablePresentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return presentMode;
        } else if (presentMode == VK_PRESENT_MODE_FIFO_KHR) {
            bestMode = presentMode;
        }
    }

    return bestMode;
}

VkExtent2D SwapChain::chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities) {
    /* Select the swapchain extent based on the window size */
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    VkExtent2D extent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };
    return extent;
}

VkImageView SwapChain::createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageView imageView;

    VkImageViewCreateInfo imageViewInfo{};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = image;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format = format;
    imageViewInfo.subresourceRange.aspectMask = aspectFlags;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.levelCount = 1;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &imageViewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image view");
    }

    return imageView;
}