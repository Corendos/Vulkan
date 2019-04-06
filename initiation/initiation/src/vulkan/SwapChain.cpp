#include "vulkan/SwapChain.hpp"

#include <array>

SwapChain::SwapChain() {}

void SwapChain::query(GLFWwindow* window,
                      vk::PhysicalDevice physicalDevice,
                      vk::Device device,
                      vk::SurfaceKHR surface) {
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
                       vk::PhysicalDevice physicalDevice,
                       vk::Device device,
                       vk::SurfaceKHR surface,
                       QueueFamilyIndices indices,
                       vk::RenderPass& renderPass) {
    if (mCreated) {
        return;
    }

    createSwapChain(window, physicalDevice, device, surface, indices);
    createImages(device);
    createImageViews(device);

    mCreated = true;
}

void SwapChain::destroy(vk::Device device) {
    if (mCreated) {
        for (auto imageView : mImagesView) {
            device.destroyImageView(imageView);
        }
        
        device.destroySwapchainKHR(mSwapChain);
        mCreated = false;
    }
}

vk::Extent2D SwapChain::getExtent() const {
    return mExtent;
}

vk::ImageView SwapChain::getImageView(uint32_t index) {
    return mImagesView[index];
}

vk::Format SwapChain::getFormat() const {
    return mSurfaceFormat.format;
}

vk::SwapchainKHR SwapChain::getHandler() const {
    return mSwapChain;
}

uint32_t SwapChain::getImageCount() const {
    return mImageCount;
}

void SwapChain::createSwapChain(GLFWwindow* window,
                                vk::PhysicalDevice physicalDevice,
                                vk::Device device,
                                vk::SurfaceKHR surface,
                                QueueFamilyIndices indices) {
    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.setSurface(surface);
    createInfo.setMinImageCount(mImageCount);
    createInfo.setImageFormat(mSurfaceFormat.format);
    createInfo.setImageColorSpace(mSurfaceFormat.colorSpace);
    createInfo.setImageExtent(mExtent);
    createInfo.setImageArrayLayers(1);
    createInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

    uint32_t queueFamilyIndices[] = {
        indices.graphicsFamily.value(),
        indices.presentFamily.value()
    };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
        createInfo.setQueueFamilyIndexCount(2);
        createInfo.setPQueueFamilyIndices(queueFamilyIndices);
    } else  {
        createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
        createInfo.setQueueFamilyIndexCount(0);
        createInfo.setPQueueFamilyIndices(nullptr);
    }

    createInfo.setPreTransform(mSwapChainSupport.capabilities.currentTransform);
    createInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
    createInfo.setPresentMode(mPresentMode);
    createInfo.setClipped(VK_TRUE);

    mSwapChain = device.createSwapchainKHR(createInfo);
}

void SwapChain::createImages(vk::Device device) {
    mImages = device.getSwapchainImagesKHR(mSwapChain);
}

void SwapChain::createImageViews(vk::Device device) {
    mImagesView.resize(mImageCount);

    for (size_t i{0}; i < mImages.size();++i) {
        vk::ImageViewCreateInfo createInfo;
        createInfo.setImage(mImages[i]);
        createInfo.setViewType(vk::ImageViewType::e2D);
        createInfo.setFormat(mSurfaceFormat.format);
        createInfo.setSubresourceRange(vk::ImageSubresourceRange(
            vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
        mImagesView[i] = device.createImageView(createInfo);
    }
}

SwapChainSupportDetails SwapChain::querySupport(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface) {
    SwapChainSupportDetails details;

    /* Get surface capabilities */
    details.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
    
    /* Get surface supported formats */
    details.formats = physicalDevice.getSurfaceFormatsKHR(surface);

    /* Get surface supported present modes */
    details.presentMode = physicalDevice.getSurfacePresentModesKHR(surface);

    return details;
}

vk::SurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    /* If the only supported format is undefined, force the format we want */
    if (availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined) {
        return {vk::Format::eR8G8B8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
    }

    /* Otherwise, try to find the format we want in the list */
    for (const auto& format : availableFormats) {
        if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return format;
        }
    }

    /* If nothing was foudn return the first format of the list */
    return availableFormats[0];
}

vk::PresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
    /* Find the best present mode */
    vk::PresentModeKHR bestMode = vk::PresentModeKHR::eImmediate;
    
    for (const auto& presentMode : availablePresentModes) {
        if (presentMode == vk::PresentModeKHR::eMailbox) {
            return presentMode;
        } else if (presentMode == vk::PresentModeKHR::eFifo) {
            bestMode = presentMode;
        }
    }

    return bestMode;
}

vk::Extent2D SwapChain::chooseSwapExtent(GLFWwindow* window, const vk::SurfaceCapabilitiesKHR& capabilities) {
    /* Select the swapchain extent based on the window size */
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    vk::Extent2D extent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };
    return extent;
}