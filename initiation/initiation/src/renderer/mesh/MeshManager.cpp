#include <iostream>
#include <array>

#include "renderer/mesh/MeshManager.hpp"
#include "vulkan/buffer/BufferHelper.hpp"
#include "tools/Profiler.hpp"

MeshManager::MeshManager() {
    mMeshes.reserve(MaximumMeshCount);
}

void MeshManager::create(VulkanContext& context) {
    mContext = &context;
    createDescriptorSetLayout();
    allocateUniformBuffer();
    allocateDescriptorSets();
}

void MeshManager::destroy() {
    mContext->getDevice().destroyDescriptorSetLayout(mRenderData.descriptorSetLayout);
    for (auto& buffers : mRenderData.renderBuffers) {
        if (buffers.vertexBufferSizeInBytes != 0)
            mContext->getMemoryManager().freeBuffer(buffers.vertexBuffer);
        if (buffers.indexBufferSizeInBytes != 0)
            mContext->getMemoryManager().freeBuffer(buffers.indexBuffer);
    }

    if (mRenderData.stagingBuffers.vertexBufferSizeInBytes != 0)
        mContext->getMemoryManager().freeBuffer(mRenderData.stagingBuffers.vertexBuffer);
    if (mRenderData.stagingBuffers.indexBufferSizeInBytes != 0)
        mContext->getMemoryManager().freeBuffer(mRenderData.stagingBuffers.indexBuffer);

    mContext->getMemoryManager().freeBuffer(mRenderData.modelTransformBuffer);

    for (size_t i{0};i < mTransferCompleteFences.size();++i) {
        mContext->getDevice().destroyFence(mTransferCompleteFences[i]);
        mContext->getDevice().destroyEvent(mEvents[i]);
    }
}

void MeshManager::addMesh(Mesh& mesh) {
    assert(mMeshes.size() + mTemporaryMeshes.size() < MaximumMeshCount);
    mTemporaryMeshes.push_back(&mesh);

    auto meshDataIt = std::find_if(mRenderData.meshDataPool.begin(), mRenderData.meshDataPool.end(),
                                     [](const MeshData& data) { return data.free; });
    assert(meshDataIt != mRenderData.meshDataPool.end());
    meshDataIt->free = false;
    mRenderData.meshDataBinding[&mesh] = &(*meshDataIt);
    updateDescriptorSet(mesh, *meshDataIt);
    mNeedStagingUpdate = true;
}

void MeshManager::removeMesh(Mesh& mesh) {
    auto meshIt = std::find(mMeshes.begin(), mMeshes.end(), &mesh);
    assert(meshIt != mMeshes.end());
    mMeshes.erase(meshIt);

    vk::DescriptorSet descriptorSet = mRenderData.meshDataBinding[&mesh]->descriptorSet;
    auto descriptorIt = std::find_if(mRenderData.meshDataPool.begin(), mRenderData.meshDataPool.end(),
                                     [&descriptorSet](const MeshData& data) { return data.descriptorSet == descriptorSet; });
    descriptorIt->free = true;
    mRenderData.meshDataBinding.erase(&mesh);
    mNeedStagingUpdate = true;
}

void MeshManager::setImageCount(uint32_t count) {
    mRenderData.renderBuffers.resize(count);
    mTransferCompleteFences.resize(count);
    mTemporaryStaticBuffers.resize(count);
    mShouldSwapBuffers.resize(count, true);
    mFirstTransfer.resize(count, true);
    mEvents.resize(count);

    for (size_t i{0};i < count;++i) {
        mTransferCompleteFences[i] = mContext->getDevice().createFence(vk::FenceCreateInfo());
        mEvents[i] = mContext->getDevice().createEvent(vk::EventCreateInfo());
    }
}

void MeshManager::update(uint32_t imageIndex) {
    if (mNeedStagingUpdate) {
        updateStagingBuffers();
    }

    if (mRenderData.renderBuffers[imageIndex].needUpdate) {
        updateStaticBuffers(imageIndex);
    }

    updateUniformBuffer();
}

vk::CommandBuffer MeshManager::render(const vk::RenderPass renderPass, const vk::Framebuffer frameBuffer, const vk::CommandPool commandPool,
                         const vk::DescriptorSet cameraDescriptorSet, const vk::PipelineLayout pipelineLayout, const vk::Pipeline pipeline,
                         uint32_t imageIndex) {
    if (mShouldSwapBuffers[imageIndex]) {
        if (mFirstTransfer[imageIndex]) {
            mContext->getDevice().waitForFences(mTransferCompleteFences[imageIndex], VK_TRUE, 1000000000);
            mFirstTransfer[imageIndex] = false;

            mShouldSwapBuffers[imageIndex] = false;

            mRenderData.renderBuffers[imageIndex] = mTemporaryStaticBuffers[imageIndex];
            while (!mTemporaryMeshes.empty()) {
                mMeshes.push_back(mTemporaryMeshes.back());
                mTemporaryMeshes.pop_back();
            }           
            mContext->getDevice().resetFences(mTransferCompleteFences[imageIndex]);
        } else {
            if (mContext->getDevice().getFenceStatus(mTransferCompleteFences[imageIndex]) == vk::Result::eSuccess) {
                mShouldSwapBuffers[imageIndex] = false;

                mRenderData.renderBuffers[imageIndex] = mTemporaryStaticBuffers[imageIndex];
                while (!mTemporaryMeshes.empty()) {
                    mMeshes.push_back(mTemporaryMeshes.back());
                    mTemporaryMeshes.pop_back();
                }

                mContext->getDevice().resetFences(mTransferCompleteFences[imageIndex]);
            }
        }
    }

    vk::CommandBufferAllocateInfo allocateInfo;
    allocateInfo.setCommandPool(commandPool);
    allocateInfo.setCommandBufferCount(1);
    allocateInfo.setLevel(vk::CommandBufferLevel::eSecondary);

    vk::CommandBuffer staticCommandBuffer = mContext->getDevice().allocateCommandBuffers(allocateInfo)[0];
    vk::CommandBufferInheritanceInfo inheritanceInfo{renderPass, 0, frameBuffer};
    staticCommandBuffer.begin(vk::CommandBufferBeginInfo(
        vk::CommandBufferUsageFlagBits::eRenderPassContinue,
        &inheritanceInfo));
    
    staticCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    staticCommandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        pipelineLayout,
        1, cameraDescriptorSet,
        {});

    staticCommandBuffer.bindVertexBuffers(
        0, mRenderData.renderBuffers[imageIndex].vertexBuffer, vk::DeviceSize());
    staticCommandBuffer.bindIndexBuffer(
        mRenderData.renderBuffers[imageIndex].indexBuffer, vk::DeviceSize(), vk::IndexType::eUint32);

    vk::DeviceSize offset{0};
    for (Mesh* mesh : mMeshes) {
        staticCommandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics, pipelineLayout,
            0, mRenderData.meshDataBinding[mesh]->descriptorSet, {});
        staticCommandBuffer.drawIndexed(mesh->getIndices().size(), 1, offset, 0, 0);
        offset += mesh->getIndices().size();
    }
    staticCommandBuffer.end();

    return staticCommandBuffer;
}

vk::DescriptorSetLayout MeshManager::getDescriptorSetLayout() const {
    return mRenderData.descriptorSetLayout;
}

void MeshManager::updateUniformBuffer() {
    void* mappingBegin;
    mContext->getMemoryManager().mapMemory(mRenderData.modelTransformBuffer,
        mRenderData.modelTransformBufferSize, &mappingBegin);
    for (Mesh* mesh : mMeshes) {
        glm::mat4 m = mesh->getTransform().getMatrix();
        void* ptr = (uint8_t*)mappingBegin + mRenderData.meshDataBinding[mesh]->uniformBufferDynamicOffset;
        memcpy(ptr, &m, sizeof(glm::mat4));
    }
    mContext->getMemoryManager().unmapMemory(mRenderData.modelTransformBuffer);
}

void MeshManager::createDescriptorSetLayout() {
    /* Create the descriptor set layout */
    vk::DescriptorSetLayoutBinding bindings[2] = {};

    /* Texture binding */
    bindings[0].setBinding(0);
    bindings[0].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
    bindings[0].setDescriptorCount(1);
    bindings[0].setStageFlags(vk::ShaderStageFlagBits::eFragment);

    /* Model matrix binding */
    bindings[1].setBinding(1);
    bindings[1].setDescriptorType(vk::DescriptorType::eUniformBuffer);
    bindings[1].setDescriptorCount(1);
    bindings[1].setStageFlags(vk::ShaderStageFlagBits::eVertex);

    vk::DescriptorSetLayoutCreateInfo createInfo{
        vk::DescriptorSetLayoutCreateFlags(), 2, bindings
    };

    mRenderData.descriptorSetLayout = mContext->getDevice().createDescriptorSetLayout(createInfo);
}

void MeshManager::allocateUniformBuffer() {
    uint32_t size;

    /* Handle the case when the render data size is less than the minimum offset alignment */
    if (sizeof(glm::mat4) > mContext->getLimits().minUniformBufferOffsetAlignment)
        size = sizeof(glm::mat4) * MaximumMeshCount;
    else
        size = mContext->getLimits().minUniformBufferOffsetAlignment * MaximumMeshCount;

    BufferHelper::createBuffer(*mContext, size, vk::BufferUsageFlagBits::eUniformBuffer,
                               vk::SharingMode::eExclusive,
                               vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                               mRenderData.modelTransformBuffer, "MeshManager::modelTransformBuffer");
    mRenderData.modelTransformBufferSize = size;
}

void MeshManager::allocateDescriptorSets() {
    std::vector<vk::DescriptorSetLayout> layouts(MaximumMeshCount, mRenderData.descriptorSetLayout);

    vk::DescriptorSetAllocateInfo allocateInfo;
    allocateInfo.setDescriptorPool(mContext->getDescriptorPool());
    allocateInfo.setDescriptorSetCount(MaximumMeshCount);
    allocateInfo.setPSetLayouts(layouts.data());

    std::vector<vk::DescriptorSet> descriptors(MaximumMeshCount);
    
    if (mContext->getDevice().allocateDescriptorSets(&allocateInfo, descriptors.data()) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to allocate descriptor sets");
    }  

    /* Copy to the mesh data pool */
    uint32_t size = sizeof(glm::mat4) < mContext->getLimits().minUniformBufferOffsetAlignment ?
        mContext->getLimits().minUniformBufferOffsetAlignment :
        sizeof(glm::mat4);

    for (size_t i{0};i < MaximumMeshCount;++i) {
        mRenderData.meshDataPool[i].descriptorSet = descriptors[i];
        mRenderData.meshDataPool[i].uniformBufferDynamicOffset = i * size;
    }
}

void MeshManager::updateStagingBuffers() {
    /* Compute buffer sizes */
    uint32_t vertexBufferSize{0}, vertexBufferSizeInBytes{0};
    uint32_t indexBufferSize{0}, indexBufferSizeInBytes{0};
    for (Mesh* mesh : mMeshes) {
        vertexBufferSize += mesh->getVertices().size();
        indexBufferSize += mesh->getIndices().size();
    }

    for (Mesh* mesh : mTemporaryMeshes) {
        vertexBufferSize += mesh->getVertices().size();
        indexBufferSize += mesh->getIndices().size();
    }
    vertexBufferSizeInBytes = vertexBufferSize * sizeof(Vertex);
    indexBufferSizeInBytes = indexBufferSize * sizeof(uint32_t);

    /* If the buffer were already allocated, free them */
    if (mRenderData.stagingBuffers.vertexBufferSizeInBytes != 0)
        mContext->getMemoryManager().freeBuffer(mRenderData.stagingBuffers.vertexBuffer);
    if (mRenderData.stagingBuffers.indexBufferSizeInBytes != 0)
        mContext->getMemoryManager().freeBuffer(mRenderData.stagingBuffers.indexBuffer);
    
    /* Allocate the device local buffers */
    BufferHelper::createBuffer(
        *mContext, vertexBufferSizeInBytes,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::SharingMode::eExclusive,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        mRenderData.stagingBuffers.vertexBuffer,
        "MeshRenderer::stagingVertexBuffer");
    BufferHelper::createBuffer(
        *mContext, indexBufferSizeInBytes,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::SharingMode::eExclusive,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        mRenderData.stagingBuffers.indexBuffer,
        "MeshRenderer::stagingIndexBuffer");
    
    /* Compute the local buffers */
    std::vector<Vertex> localVertexBuffer(vertexBufferSize);
    std::vector<uint32_t> localIndexBuffer(indexBufferSize);

    auto vertexIterator = localVertexBuffer.begin();
    auto indexIterator = localIndexBuffer.begin();
    uint32_t indexOffset{0};
    for (Mesh* mesh : mMeshes) {
        vertexIterator = std::copy(mesh->getVertices().begin(), mesh->getVertices().end(),
                                   vertexIterator);

        indexIterator = std::transform(
            mesh->getIndices().begin(), mesh->getIndices().end(),
            indexIterator, [indexOffset](const uint32_t& i) { return i + indexOffset; });
        indexOffset += mesh->getVertices().size();
    }

    for (Mesh* mesh : mTemporaryMeshes) {
        vertexIterator = std::copy(mesh->getVertices().begin(), mesh->getVertices().end(),
                                   vertexIterator);

        indexIterator = std::transform(
            mesh->getIndices().begin(), mesh->getIndices().end(),
            indexIterator, [indexOffset](const uint32_t& i) { return i + indexOffset; });
        indexOffset += mesh->getVertices().size();
    }

    /* Copy the buffers */
    void* data;
    mContext->getMemoryManager().mapMemory(mRenderData.stagingBuffers.vertexBuffer, vertexBufferSizeInBytes, &data);
    memcpy(data, localVertexBuffer.data(), vertexBufferSizeInBytes);
    mContext->getMemoryManager().unmapMemory(mRenderData.stagingBuffers.vertexBuffer);

    mContext->getMemoryManager().mapMemory(mRenderData.stagingBuffers.indexBuffer, indexBufferSizeInBytes, &data);
    memcpy(data, localIndexBuffer.data(), indexBufferSizeInBytes);
    mContext->getMemoryManager().unmapMemory(mRenderData.stagingBuffers.indexBuffer);

    mRenderData.stagingBuffers.vertexBufferSize = vertexBufferSize;
    mRenderData.stagingBuffers.vertexBufferSizeInBytes = vertexBufferSizeInBytes;
    mRenderData.stagingBuffers.indexBufferSize = indexBufferSize;
    mRenderData.stagingBuffers.indexBufferSizeInBytes = indexBufferSizeInBytes;

    for (auto& buffers : mRenderData.renderBuffers) {
        buffers.needUpdate = true;
    }

    mNeedStagingUpdate = false;
}

void MeshManager::updateDescriptorSet(Mesh& mesh, MeshData& meshData) {
    /* Texture info */
    vk::DescriptorImageInfo info{
        mesh.getTexture().getSampler(),
        mesh.getTexture().getImageView(),
        vk::ImageLayout::eShaderReadOnlyOptimal};

    /* Uniform buffer info */
    vk::DescriptorBufferInfo bufferInfo{
        mRenderData.modelTransformBuffer,
        meshData.uniformBufferDynamicOffset,
        sizeof(glm::mat4)};

    std::array<vk::WriteDescriptorSet, 2> writes;
    writes[0].setDescriptorCount(1);
    writes[0].setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
    writes[0].setDstBinding(0);
    writes[0].setDstSet(meshData.descriptorSet);
    writes[0].setPImageInfo(&info);

    writes[1].setDescriptorCount(1);
    writes[1].setDescriptorType(vk::DescriptorType::eUniformBuffer);
    writes[1].setDstBinding(1);
    writes[1].setDstSet(meshData.descriptorSet);
    writes[1].setPBufferInfo(&bufferInfo);

    mContext->getDevice().updateDescriptorSets(writes, {});
}

void MeshManager::updateStaticBuffers(uint32_t imageIndex) {
    RenderBuffers renderBuffer;
    renderBuffer.vertexBufferSize = mRenderData.stagingBuffers.vertexBufferSize;
    renderBuffer.vertexBufferSizeInBytes = mRenderData.stagingBuffers.vertexBufferSizeInBytes;
    renderBuffer.indexBufferSize = mRenderData.stagingBuffers.indexBufferSize;
    renderBuffer.indexBufferSizeInBytes = mRenderData.stagingBuffers.indexBufferSizeInBytes;
    renderBuffer.needUpdate = false;
    BufferHelper::createBuffer(
        *mContext, mRenderData.stagingBuffers.vertexBufferSizeInBytes,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
        vk::SharingMode::eExclusive,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        renderBuffer.vertexBuffer,
        "MeshRenderer::vertexBuffer");
    BufferHelper::createBuffer(
        *mContext, mRenderData.stagingBuffers.indexBufferSizeInBytes,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
        vk::SharingMode::eExclusive,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        renderBuffer.indexBuffer,
        "MeshRenderer::indexBuffer");

    vk::CommandBuffer transferCommandBuffer = mContext->getDevice().allocateCommandBuffers(
        vk::CommandBufferAllocateInfo(mContext->getTransferCommandPool(),
            vk::CommandBufferLevel::ePrimary, 1))[0];

    vk::BufferCopy region{0, 0, mRenderData.stagingBuffers.vertexBufferSizeInBytes};
    transferCommandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    transferCommandBuffer.copyBuffer(mRenderData.stagingBuffers.vertexBuffer, renderBuffer.vertexBuffer,
        region);
    region.setSize(mRenderData.stagingBuffers.indexBufferSizeInBytes);
    transferCommandBuffer.copyBuffer(mRenderData.stagingBuffers.indexBuffer, renderBuffer.indexBuffer,
        region);  
    transferCommandBuffer.end();

    vk::SubmitInfo submitInfo{
        0, nullptr, nullptr,
        1, &transferCommandBuffer
    };
    mContext->getGraphicsQueue().submit(submitInfo, mTransferCompleteFences[imageIndex]);

    mTemporaryStaticBuffers[imageIndex] = renderBuffer;
    mShouldSwapBuffers[imageIndex] = true;
}