#include "renderer/Renderer.hpp"

#include <chrono>
#include <iostream>
#include <array>

#include "vulkan/UniformBufferObject.hpp"
#include "vulkan/Commands.hpp"
#include "vulkan/buffer/BufferHelper.hpp"
#include "vulkan/image/ImageHelper.hpp"
#include "colors/Color.hpp"
#include "renderer/RenderInfo.hpp"
#include "environment.hpp"

Renderer::Renderer() {
    mClearValues[0].color = std::array<float, 4>({0.325f, 0.694f, 0.937f, 1.0f});
    mClearValues[1].color = vk::ClearColorValue();
    mClearValues[2].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

    mNextImageIndex = 0;
}

void Renderer::create(VulkanContext& context, TextureManager& textureManager, MeshManager& meshManager) {
    if (mCreated) {
        return;
    }

    mContext = &context;
    mTextureManager = &textureManager;
    mMeshManager = &meshManager;

    mSwapChain.query(mContext->getWindow(),
                     mContext->getPhysicalDevice(),
                     mContext->getDevice(),
                     mContext->getSurface());
    mExtent = mSwapChain.getExtent();
    createRenderPass();
    mSwapChain.create(mContext->getWindow(),
                      mContext->getPhysicalDevice(),
                      mContext->getDevice(),
                      mContext->getSurface(),
                      mContext->getQueueFamilyIndices(),
                      mRenderPass);
    createFramebuffers();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createDescriptorPool();
    createCameraUniformBuffers();
    createCameraDescriptorSets();
    createCommandPools();
    createCommandBuffers();
    createSemaphores();
    createFences();

    mCreated = true;
}

void Renderer::recreate() {
    int width{0}, height{0};
    while(width == 0 || height == 0) {
        glfwGetFramebufferSize(mContext->getWindow(), &width, &height);
        glfwWaitEvents();
    }
    
    vkDeviceWaitIdle(mContext->getDevice());

    /* Destroying resources that need to be recreated */

    for (auto& framebufferAttachment : mFramebufferAttachments) {
        mContext->getDevice().destroyImage(framebufferAttachment.normal.image);
        mContext->getDevice().destroyImageView(framebufferAttachment.normal.imageView);
        mContext->getDevice().destroyImage(framebufferAttachment.depth.image);
        mContext->getDevice().destroyImageView(framebufferAttachment.depth.imageView);
    }

    for (auto& framebuffer : mFrameBuffers) {
        mContext->getDevice().destroyFramebuffer(framebuffer);
    }
    
    mSwapChain.destroy(mContext->getDevice());
    mContext->getDevice().destroyRenderPass(mRenderPass);
    mContext->getDevice().destroyPipeline(mPipeline);
    mContext->getDevice().destroyPipelineLayout(mPipelineLayout);

    /* Recreate the resources */

    mSwapChain.query(mContext->getWindow(), mContext->getPhysicalDevice(), mContext->getDevice(), mContext->getSurface());
    mExtent = mSwapChain.getExtent();
    createRenderPass();
    mSwapChain.create(mContext->getWindow(), mContext->getPhysicalDevice(), mContext->getDevice(),
                      mContext->getSurface(), mContext->getQueueFamilyIndices(),
                      mRenderPass);
    createFramebuffers();
    createGraphicsPipeline();
    createCommandBuffers();
    mCamera->setExtent(mSwapChain.getExtent());
}

void Renderer::destroy() {
    if (mCreated) {
        for (size_t i{0};i < mSwapChain.getImageCount();++i) {
            mContext->getMemoryManager().freeBuffer(mCameraUniformBuffers[i]);
        }

        for (auto& framebufferAttachment : mFramebufferAttachments) {
        mContext->getDevice().destroyImage(framebufferAttachment.normal.image);
        mContext->getDevice().destroyImageView(framebufferAttachment.normal.imageView);
        mContext->getDevice().destroyImage(framebufferAttachment.depth.image);
        mContext->getDevice().destroyImageView(framebufferAttachment.depth.imageView);
        }

        for (auto& framebuffer : mFrameBuffers) {
        mContext->getDevice().destroyFramebuffer(framebuffer);
        }

        mContext->getDevice().destroyShaderModule(mVertexShader.first);
        mContext->getDevice().destroyShaderModule(mFragmentShader.first);

        for (auto& commandPool : mCommandPools) {
            vkDestroyCommandPool(mContext->getDevice(), commandPool, nullptr);
        }

        vkDestroyDescriptorPool(mContext->getDevice(), mDescriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(mContext->getDevice(), mCameraDescriptorSetLayout, nullptr);

        mSwapChain.destroy(mContext->getDevice());
        mContext->getDevice().destroyRenderPass(mRenderPass);
        mContext->getDevice().destroyPipeline(mPipeline);
        mContext->getDevice().destroyPipelineLayout(mPipelineLayout);
        
        vkDestroySemaphore(mContext->getDevice(), mImageAvailableSemaphore, nullptr);
        for (size_t i{0};i < mRenderFinishedSemaphores.size();++i) {
            vkDestroySemaphore(mContext->getDevice(), mRenderFinishedSemaphores[i], nullptr);
        }

        for (size_t i{0};i < mFencesInfo.size();++i) {
            vkDestroyFence(mContext->getDevice(), mFencesInfo[i].fence, nullptr);
        }

        mCreated = false;
    }
}

void Renderer::render() {
    if (mBypassRendering) {
        return;
    }

    mToWaitSemaphores.push_back(mImageAvailableSemaphore);
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

    vk::SubmitInfo submitInfo;
    submitInfo.setWaitSemaphoreCount(mToWaitSemaphores.size());
    submitInfo.setPWaitSemaphores(mToWaitSemaphores.data());
    submitInfo.setPWaitDstStageMask(waitStages);
    submitInfo.setCommandBufferCount(1);
    submitInfo.setPCommandBuffers(&mCommandBuffers[mNextImageIndex]);

    vk::Semaphore signalSemaphores[] = {mRenderFinishedSemaphores[mNextImageIndex]};
    submitInfo.setSignalSemaphoreCount(1);
    submitInfo.setPSignalSemaphores(signalSemaphores);

    mContext->getGraphicsQueue().submit(submitInfo, mFencesInfo[mNextImageIndex].fence);

    mFencesInfo[mNextImageIndex].submitted = true;

    vk::SwapchainKHR swapChains[] = {mSwapChain.getHandler()};

    vk::PresentInfoKHR presentInfo;
    presentInfo.setWaitSemaphoreCount(1);
    presentInfo.setPWaitSemaphores(signalSemaphores);
    presentInfo.setSwapchainCount(1);
    presentInfo.setPSwapchains(swapChains);
    presentInfo.setPImageIndices(&mNextImageIndex);

    vk::Result result = mContext->getPresentQueue().presentKHR(presentInfo);
    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
        recreate();
    } else if (result != vk::Result::eSuccess) {
        throw std::runtime_error("  Failed to present image");
    }
}

void Renderer::update(double dt) {
    acquireNextImage();

    mToWaitSemaphores.clear();

    mMeshManager->update(mNextImageIndex);

    vk::CommandBuffer staticBuffer = mMeshManager->render(
        mRenderPass, mFrameBuffers[mNextImageIndex],
        mCommandPools[mNextImageIndex], mCameraDescriptorSets[mNextImageIndex],
        mPipelineLayout, mPipeline, mNextImageIndex);

    vk::CommandBufferAllocateInfo allocateInfo;
    allocateInfo.setCommandPool(mCommandPools[mNextImageIndex]);
    allocateInfo.setCommandBufferCount(1);
    allocateInfo.setLevel(vk::CommandBufferLevel::ePrimary);

    mCommandBuffers[mNextImageIndex] = mContext->getDevice().allocateCommandBuffers(allocateInfo)[0];

    mCommandBuffers[mNextImageIndex].begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.setRenderPass(mRenderPass);
    renderPassInfo.setFramebuffer(mFrameBuffers[mNextImageIndex]);
    renderPassInfo.setRenderArea(vk::Rect2D({0, 0}, mSwapChain.getExtent()));
    renderPassInfo.setClearValueCount(mClearValues.size());
    renderPassInfo.setPClearValues(mClearValues.data());

    mCommandBuffers[mNextImageIndex].beginRenderPass(renderPassInfo,vk::SubpassContents::eSecondaryCommandBuffers);

    mCommandBuffers[mNextImageIndex].executeCommands(staticBuffer);

    mCommandBuffers[mNextImageIndex].endRenderPass();

    mCommandBuffers[mNextImageIndex].end();

    updateUniformBuffer(mNextImageIndex);
    waitForFence();
}

void Renderer::setCamera(Camera& camera) {
    mCamera = &camera;
}

void Renderer::setLight(Light& light) {
    mLight = &light;
}

vk::DescriptorPool Renderer::getDescriptorPool() const {
    return mDescriptorPool;
}

SwapChain& Renderer::getSwapChain() {
    return mSwapChain;
}

void Renderer::acquireNextImage() {
    mBypassRendering = false;
    vk::ResultValue<uint32_t> resultValue = mContext->getDevice().acquireNextImageKHR(
        mSwapChain.getHandler(), std::numeric_limits<uint64_t>::max() - 1,
        mImageAvailableSemaphore, vk::Fence());

    if (resultValue.result == vk::Result::eErrorOutOfDateKHR) {
        recreate();
        mBypassRendering = true;
        return;
    } else if (resultValue.result != vk::Result::eSuccess && resultValue.result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("Failed to acquire swap chain image");
    } else {
        mNextImageIndex = resultValue.value;
    }
}

void Renderer::waitForFence() {
    if (mFencesInfo[mNextImageIndex].submitted) {
        if (mContext->getDevice().getFenceStatus(mFencesInfo[mNextImageIndex].fence) == vk::Result::eNotReady) {
            mContext->getDevice().waitForFences(mFencesInfo[mNextImageIndex].fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
        }
        mContext->getDevice().resetFences(mFencesInfo[mNextImageIndex].fence);
    }
}

void Renderer::createRenderPass() {
    /* Create framebuffers */
    mFramebufferAttachments.resize(mSwapChain.getImageCount());
    
    vk::MemoryRequirements memoryRequirements;
    for (auto& framebufferAttachment : mFramebufferAttachments) {
        /* Create the normal attachment */
        framebufferAttachment.normal = createAttachment(
            vk::Format::eR8G8B8A8Unorm,
            vk::ImageUsageFlagBits::eColorAttachment,
            vk::ImageAspectFlagBits::eColor);
        
        /* Create the depth attachment */
        framebufferAttachment.depth = createAttachment(
            findDepthFormat(),
            vk::ImageUsageFlagBits::eDepthStencilAttachment,
            vk::ImageAspectFlagBits::eDepth);
    }

    /* Create the render pass attachments */
    std::array<vk::AttachmentDescription, 3> attachments;
    attachments[0].setFormat(mSwapChain.getFormat());
    attachments[0].setSamples(vk::SampleCountFlagBits::e1);
    attachments[0].setLoadOp(vk::AttachmentLoadOp::eClear);
    attachments[0].setStoreOp(vk::AttachmentStoreOp::eStore);
    attachments[0].setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    attachments[0].setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    attachments[0].setInitialLayout(vk::ImageLayout::eUndefined);
    attachments[0].setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    attachments[1].setFormat(vk::Format::eR8G8B8A8Unorm);
    attachments[1].setSamples(vk::SampleCountFlagBits::e1);
    attachments[1].setLoadOp(vk::AttachmentLoadOp::eClear);
    attachments[1].setStoreOp(vk::AttachmentStoreOp::eStore);
    attachments[1].setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    attachments[1].setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    attachments[1].setInitialLayout(vk::ImageLayout::eUndefined);
    attachments[1].setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

    attachments[2].setFormat(findDepthFormat());
    attachments[2].setSamples(vk::SampleCountFlagBits::e1);
    attachments[2].setLoadOp(vk::AttachmentLoadOp::eClear);
    attachments[2].setStoreOp(vk::AttachmentStoreOp::eDontCare);
    attachments[2].setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
    attachments[2].setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    attachments[2].setInitialLayout(vk::ImageLayout::eUndefined);
    attachments[2].setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);


    std::array<vk::AttachmentReference, 2> colorAttachementReferences = {{
        {0, vk::ImageLayout::eColorAttachmentOptimal},
        {1, vk::ImageLayout::eColorAttachmentOptimal}
    }};

    vk::AttachmentReference depthAttachmentReference{
        2, vk::ImageLayout::eDepthStencilAttachmentOptimal};

    vk::SubpassDescription subpassDescription;
    subpassDescription.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
    subpassDescription.setColorAttachmentCount(2);
    subpassDescription.setPColorAttachments(colorAttachementReferences.data());
    subpassDescription.setPDepthStencilAttachment(&depthAttachmentReference);

    vk::SubpassDependency dependency{VK_SUBPASS_EXTERNAL, 0,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::AccessFlags(),
        vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite};
    
    vk::RenderPassCreateInfo createInfo;
    createInfo.setAttachmentCount(attachments.size());
    createInfo.setPAttachments(attachments.data());
    createInfo.setDependencyCount(1);
    createInfo.setPDependencies(&dependency);
    createInfo.setSubpassCount(1);
    createInfo.setPSubpasses(&subpassDescription);
    
    mRenderPass = mContext->getDevice().createRenderPass(createInfo);
}

void Renderer::createGraphicsPipeline() {
    mVertexShader = Shader::create(
        *mContext, shaderPath + "vert.spv", vk::ShaderStageFlagBits::eVertex, "main");
    mFragmentShader = Shader::create(
        *mContext, shaderPath + "frag.spv", vk::ShaderStageFlagBits::eFragment, "main");

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
        mMeshManager->getDescriptorSetLayout(),
        mCameraDescriptorSetLayout
    };

    vk::PipelineLayoutCreateInfo createInfo;
    createInfo.setSetLayoutCount(descriptorSetLayouts.size());
    createInfo.setPSetLayouts(descriptorSetLayouts.data());

    mPipelineLayout = mContext->getDevice().createPipelineLayout(createInfo);

    mPipeline = GraphicsPipeline::create(
        *mContext, mRenderPass, mExtent,
        mPipelineLayout, mVertexShader.second, mFragmentShader.second);
}

void Renderer::createFramebuffers() {
    mFrameBuffers.resize(mSwapChain.getImageCount());


    vk::FramebufferCreateInfo createInfo;
    createInfo.setRenderPass(mRenderPass);
    createInfo.setWidth(mExtent.width);
    createInfo.setHeight(mExtent.height);
    createInfo.setLayers(1);

    for (size_t i{0}; i < mSwapChain.getImageCount();++i) {
        std::vector<vk::ImageView> attachments = {
            mSwapChain.getImageView(i),
            mFramebufferAttachments[i].normal.imageView,
            mFramebufferAttachments[i].depth.imageView,
        };

        createInfo.setAttachmentCount(attachments.size());
        createInfo.setPAttachments(attachments.data());
        mFrameBuffers[i] = mContext->getDevice().createFramebuffer(createInfo);
    }
}

void Renderer::createDescriptorPool() {
    std::array<vk::DescriptorPoolSize, 2> poolSizes;
    poolSizes[0].setType(vk::DescriptorType::eUniformBuffer);
    poolSizes[0].setDescriptorCount(10000);
    poolSizes[1].setType(vk::DescriptorType::eCombinedImageSampler);
    poolSizes[1].setDescriptorCount(10000);

    vk::DescriptorPoolCreateInfo poolInfo;
    poolInfo.setPoolSizeCount(poolSizes.size());
    poolInfo.setPPoolSizes(poolSizes.data());
    poolInfo.setMaxSets(10000);

    mDescriptorPool = mContext->getDevice().createDescriptorPool(poolInfo);
}

void Renderer::createDescriptorSetLayout() {
    vk::DescriptorSetLayoutBinding cameraLayoutBinding;
    cameraLayoutBinding.setBinding(0);
    cameraLayoutBinding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
    cameraLayoutBinding.setDescriptorCount(1);
    cameraLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eVertex);

    vk::DescriptorSetLayoutCreateInfo cameraLayoutInfo;
    cameraLayoutInfo.setBindingCount(1);
    cameraLayoutInfo.setPBindings(&cameraLayoutBinding);

    mCameraDescriptorSetLayout = mContext->getDevice().createDescriptorSetLayout(cameraLayoutInfo);
}

void Renderer::createCommandBuffers() {
    mCommandBuffers.resize(mSwapChain.getImageCount());
}

void Renderer::createCameraUniformBuffers() {
    mCameraUniformBuffers.resize(mSwapChain.getImageCount());
    vk::DeviceSize bufferSize = sizeof(RenderInfo);

    for (size_t i{0};i < mCameraUniformBuffers.size();++i) {
        BufferHelper::createBuffer(*mContext,
                               bufferSize,
                               vk::BufferUsageFlagBits::eUniformBuffer,
                               vk::SharingMode::eExclusive,
                               vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostCached,
                               mCameraUniformBuffers[i],
                               "Camera Uniform Buffer");
    }
}

void Renderer::createCameraDescriptorSets() {
    mCameraDescriptorSets.resize(mSwapChain.getImageCount());

    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.setDescriptorPool(mDescriptorPool);
    allocInfo.setDescriptorSetCount(1);
    allocInfo.setPSetLayouts(&mCameraDescriptorSetLayout);

    // TODO: switch to a one shot allocation
    for (size_t i{0};i < mCameraDescriptorSets.size();++i) {
        mCameraDescriptorSets[i] = mContext->getDevice().allocateDescriptorSets(allocInfo)[0];
    }

    for (size_t i{0};i < mCameraDescriptorSets.size();++i) {
        vk::DescriptorBufferInfo bufferInfo{mCameraUniformBuffers[i], 0, sizeof(RenderInfo)};

        vk::WriteDescriptorSet descriptorWrite;
        descriptorWrite.setDstSet(mCameraDescriptorSets[i]);
        descriptorWrite.setDstBinding(0);
        descriptorWrite.setDescriptorType(vk::DescriptorType::eUniformBuffer);
        descriptorWrite.setDescriptorCount(1);
        descriptorWrite.setPBufferInfo(&bufferInfo);

        mContext->getDevice().updateDescriptorSets(descriptorWrite, {});
    }
}

void Renderer::createSemaphores() {
    mRenderFinishedSemaphores.resize(mSwapChain.getImageCount());

    for (size_t i{0};i < mSwapChain.getImageCount();++i) {
        mRenderFinishedSemaphores[i] = mContext->getDevice().createSemaphore(vk::SemaphoreCreateInfo());
    }

    mImageAvailableSemaphore = mContext->getDevice().createSemaphore(vk::SemaphoreCreateInfo());
}

void Renderer::createFences() {
    mFencesInfo.resize(mSwapChain.getImageCount());

    for (size_t i{0};i < mFencesInfo.size();++i) {
        mFencesInfo[i].fence = mContext->getDevice().createFence(vk::FenceCreateInfo());
    }
}

void Renderer::createCommandPools() {
    mCommandPools.resize(mSwapChain.getImageCount());

    vk::CommandPoolCreateInfo createInfo{vk::CommandPoolCreateFlags(),
        mContext->getQueueFamilyIndices().graphicsFamily.value()};

    for (size_t i{0};i < mSwapChain.getImageCount();++i) {
        mCommandPools[i] = mContext->getDevice().createCommandPool(createInfo);
    }
}

FrameBufferAttachment Renderer::createAttachment(vk::Format format,
                                                 vk::ImageUsageFlags usage,
                                                 vk::ImageAspectFlags aspect) {
    FrameBufferAttachment attachment;

    vk::ImageCreateInfo createInfo;
    createInfo.setImageType(vk::ImageType::e2D);
    createInfo.setFormat(format);
    createInfo.setExtent({mExtent.width, mExtent.height, 1});
    createInfo.setMipLevels(1);
    createInfo.setArrayLayers(1);
    createInfo.setSamples(vk::SampleCountFlagBits::e1);
    createInfo.setTiling(vk::ImageTiling::eOptimal);
    createInfo.setUsage(usage);
    createInfo.setInitialLayout(vk::ImageLayout::eUndefined);
    attachment.image = mContext->getDevice().createImage(createInfo);

    vk::MemoryRequirements memoryRequirements = mContext->getDevice().getImageMemoryRequirements(attachment.image);

    mContext->getMemoryManager().allocateForImage(
        attachment.image,
        memoryRequirements,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        "attachment"
    );

    vk::ImageViewCreateInfo createInfo2;

    createInfo2.setViewType(vk::ImageViewType::e2D);
    createInfo2.setFormat(format);
    createInfo2.setSubresourceRange(
        vk::ImageSubresourceRange(aspect, 0, 1, 0, 1));
    createInfo2.setImage(attachment.image);
    attachment.imageView = mContext->getDevice().createImageView(createInfo2);

    return attachment;
}

void Renderer::updateUniformBuffer(uint32_t index) {
    RenderInfo renderInfo;
    renderInfo.proj = mCamera->getProj();
    renderInfo.view = mCamera->getView();
    renderInfo.cameraPosition = glm::vec4(mCamera->getPosition(), 1.0);
    renderInfo.lightPosition = glm::vec4(mLight->position, 1.0);

    void* data;
    mContext->getMemoryManager().mapMemory(mCameraUniformBuffers[index], sizeof(RenderInfo), &data);
    memcpy(data, &renderInfo, sizeof(RenderInfo));
    mContext->getMemoryManager().unmapMemory(mCameraUniformBuffers[index]);
}

vk::Format Renderer::findSupportedFormat(const std::vector<vk::Format>& candidates,
                                         vk::ImageTiling tiling,
                                         vk::FormatFeatureFlags features) {
    for (vk::Format format : candidates) {
        vk::FormatProperties properties = mContext->getPhysicalDevice().getFormatProperties(format);

        if (tiling == vk::ImageTiling::eLinear && (properties.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == vk::ImageTiling::eOptimal && (properties.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format");
}

vk::Format Renderer::findDepthFormat() {
    return findSupportedFormat(
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment
    );
}