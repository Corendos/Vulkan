#include <iostream>

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
    vkDestroyDescriptorSetLayout(mContext->getDevice(), mRenderData.descriptorSetLayout, nullptr);
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
        vkDestroyFence(mContext->getDevice(), mTransferCompleteFences[i], nullptr);
        vkDestroyEvent(mContext->getDevice(), mEvents[i], nullptr);
    }
}

void MeshManager::addMesh(Mesh& mesh) {
    assert(mMeshes.size() < MaximumMeshCount);
    mMeshes.push_back(&mesh);

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

    VkDescriptorSet descriptorSet = mRenderData.meshDataBinding[&mesh]->descriptorSet;
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

    VkFenceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    VkEventCreateInfo createInfo2{};
    createInfo2.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;

    for (size_t i{0};i < count;++i) {
        if (vkCreateFence(mContext->getDevice(), &createInfo, nullptr, &mTransferCompleteFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("Error, failed to create fence");
        }

        if (vkCreateEvent(mContext->getDevice(), &createInfo2, nullptr, &mEvents[i]) != VK_SUCCESS) {
            throw std::runtime_error("Error, failed to create event");
        }
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

VkCommandBuffer MeshManager::render(const VkRenderPass renderPass, const VkFramebuffer frameBuffer, const VkCommandPool commandPool,
                         const VkDescriptorSet cameraDescriptorSet, const VkPipelineLayout pipelineLayout, const VkPipeline pipeline,
                         uint32_t imageIndex) {
    if (mShouldSwapBuffers[imageIndex]) {
        if (mFirstTransfer[imageIndex]) {
            vkWaitForFences(mContext->getDevice(), 1, &mTransferCompleteFences[imageIndex], VK_TRUE, 1000000000);
            mFirstTransfer[imageIndex] = false;

            mRenderData.renderBuffers[imageIndex] = mTemporaryStaticBuffers[imageIndex];
            mShouldSwapBuffers[imageIndex] = false;

            vkResetFences(mContext->getDevice(), 1, &mTransferCompleteFences[imageIndex]);
        } else {
            if (vkGetFenceStatus(mContext->getDevice(), mTransferCompleteFences[imageIndex]) == VK_SUCCESS) {
                mRenderData.renderBuffers[imageIndex] = mTemporaryStaticBuffers[imageIndex];
                mShouldSwapBuffers[imageIndex] = false;

                vkResetFences(mContext->getDevice(), 1, &mTransferCompleteFences[imageIndex]);
            }
        }
    }
    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = commandPool;
    allocateInfo.commandBufferCount = 1;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;

    VkCommandBuffer staticCommandBuffer;

    vkAllocateCommandBuffers(mContext->getDevice(), &allocateInfo, &staticCommandBuffer);

    VkCommandBufferInheritanceInfo inheritanceInfo{};
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.renderPass = renderPass;
    inheritanceInfo.subpass = 0;
    inheritanceInfo.framebuffer = frameBuffer;
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    beginInfo.pInheritanceInfo = &inheritanceInfo;

    vkBeginCommandBuffer(staticCommandBuffer, &beginInfo);

    vkCmdBindPipeline(staticCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    
    vkCmdBindDescriptorSets(staticCommandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout,
                            1, 1, &cameraDescriptorSet,
                            0, nullptr);


    uint32_t offset{0};
    VkDeviceSize vertexOffset{0};
    vkCmdBindVertexBuffers(staticCommandBuffer, 0, 1, &mRenderData.renderBuffers[imageIndex].vertexBuffer, &vertexOffset);
    vkCmdBindIndexBuffer(staticCommandBuffer, mRenderData.renderBuffers[imageIndex].indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    for (Mesh* mesh : mMeshes) {
        vkCmdBindDescriptorSets(staticCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout, 0, 1, &mRenderData.meshDataBinding[mesh]->descriptorSet,
            0, nullptr);
        vkCmdDrawIndexed(staticCommandBuffer, mesh->getIndices().size(), 1, offset, 0, 0);
        offset += mesh->getIndices().size();
    }

    vkEndCommandBuffer(staticCommandBuffer);

    return staticCommandBuffer;
}

VkDescriptorSetLayout MeshManager::getDescriptorSetLayout() const {
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
    VkDescriptorSetLayoutBinding bindings[2] = {};

    /* Texture binding */
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    /* Model matrix binding */
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = 2;
    createInfo.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(mContext->getDevice(), &createInfo, nullptr, &mRenderData.descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout");
    }
}

void MeshManager::allocateUniformBuffer() {
    uint32_t size;

    /* Handle the case when the render data size is less than the minimum offset alignment */
    if (sizeof(glm::mat4) > mContext->getLimits().minUniformBufferOffsetAlignment)
        size = sizeof(glm::mat4) * MaximumMeshCount;
    else
        size = mContext->getLimits().minUniformBufferOffsetAlignment * MaximumMeshCount;

    BufferHelper::createBuffer(*mContext, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               VK_SHARING_MODE_EXCLUSIVE,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                               mRenderData.modelTransformBuffer, "MeshManager::modelTransformBuffer");
    mRenderData.modelTransformBufferSize = size;
}

void MeshManager::allocateDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(MaximumMeshCount, mRenderData.descriptorSetLayout);

    VkDescriptorSetAllocateInfo infos{};
    infos.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    infos.descriptorPool = mContext->getDescriptorPool().getHandler();
    infos.descriptorSetCount = MaximumMeshCount;
    infos.pSetLayouts = layouts.data();

    std::vector<VkDescriptorSet> descriptors(MaximumMeshCount, VK_NULL_HANDLE);

    if (vkAllocateDescriptorSets(mContext->getDevice(), &infos, descriptors.data()) != VK_SUCCESS) {
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
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        mRenderData.stagingBuffers.vertexBuffer,
        "MeshRenderer::stagingVertexBuffer");
    BufferHelper::createBuffer(
        *mContext, indexBufferSizeInBytes,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
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

    /* Copy the buffers */
    void* data;
    mContext->getMemoryManager().mapMemory(mRenderData.stagingBuffers.vertexBuffer, vertexBufferSizeInBytes, &data);
    memcpy(data, localVertexBuffer.data(), vertexBufferSizeInBytes);
    mContext->getMemoryManager().unmapMemory(mRenderData.stagingBuffers.vertexBuffer);

    mContext->getMemoryManager().mapMemory(mRenderData.stagingBuffers.indexBuffer, indexBufferSizeInBytes, &data);
    memcpy(data, localIndexBuffer.data(), indexBufferSizeInBytes);
    mContext->getMemoryManager().unmapMemory(mRenderData.stagingBuffers.indexBuffer);

    for (size_t i{0};i < mEvents.size();++i) {
        vkSetEvent(mContext->getDevice(), mEvents[i]);
    }

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
    VkDescriptorImageInfo info{};
    info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    info.imageView = mesh.getTexture().getImageView().getHandler();
    info.sampler = mesh.getTexture().getSampler().getHandler();

    /* Uniform buffer info */
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = mRenderData.modelTransformBuffer;
    bufferInfo.offset = meshData.uniformBufferDynamicOffset;
    bufferInfo.range = sizeof(glm::mat4);

    VkWriteDescriptorSet writes[2] = {};
    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[0].dstBinding = 0;
    writes[0].dstSet = meshData.descriptorSet;
    writes[0].pImageInfo = &info;

    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].descriptorCount = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[1].dstBinding = 1;
    writes[1].dstSet = meshData.descriptorSet;
    writes[1].pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(mContext->getDevice(), 2, writes, 0, nullptr);
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
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        renderBuffer.vertexBuffer,
        "MeshRenderer::vertexBuffer");
    BufferHelper::createBuffer(
        *mContext, mRenderData.stagingBuffers.indexBufferSizeInBytes,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        renderBuffer.indexBuffer,
        "MeshRenderer::indexBuffer");

    VkCommandBuffer transferCommandBuffer;

    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;
    allocateInfo.commandPool = mContext->getTransferCommandPool().getHandler();

    if (vkAllocateCommandBuffers(mContext->getDevice(), &allocateInfo, &transferCommandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffer");
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkBufferCopy region{};
    region.dstOffset = 0;
    region.srcOffset = 0;
    region.size = mRenderData.stagingBuffers.vertexBufferSizeInBytes;

    VkBufferMemoryBarrier memoryBarriers[2] = {};
    memoryBarriers[0].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    memoryBarriers[0].srcAccessMask = VK_PIPELINE_STAGE_HOST_BIT;
    memoryBarriers[0].dstAccessMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    memoryBarriers[0].buffer = renderBuffer.vertexBuffer;
    memoryBarriers[0].offset = 0;
    memoryBarriers[0].size = renderBuffer.vertexBufferSizeInBytes;
    memoryBarriers[0].srcQueueFamilyIndex = mContext->getQueueFamilyIndices().graphicsFamily.value();
    memoryBarriers[0].dstQueueFamilyIndex = mContext->getQueueFamilyIndices().graphicsFamily.value();

    memoryBarriers[1].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    memoryBarriers[1].srcAccessMask = VK_PIPELINE_STAGE_HOST_BIT;
    memoryBarriers[1].dstAccessMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    memoryBarriers[1].buffer = renderBuffer.indexBuffer;
    memoryBarriers[1].offset = 0;
    memoryBarriers[1].size = renderBuffer.indexBufferSizeInBytes;
    memoryBarriers[1].srcQueueFamilyIndex = mContext->getQueueFamilyIndices().graphicsFamily.value();
    memoryBarriers[1].dstQueueFamilyIndex = mContext->getQueueFamilyIndices().graphicsFamily.value();
    

    vkBeginCommandBuffer(transferCommandBuffer, &beginInfo);
    vkCmdWaitEvents(transferCommandBuffer, 1, &mEvents[imageIndex],
                    VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                    0, nullptr,
                    2, memoryBarriers,
                    0, nullptr);
    vkCmdCopyBuffer(transferCommandBuffer, mRenderData.stagingBuffers.vertexBuffer, renderBuffer.vertexBuffer,
                    1, &region);
    region.size = mRenderData.stagingBuffers.indexBufferSizeInBytes;
    vkCmdCopyBuffer(transferCommandBuffer, mRenderData.stagingBuffers.indexBuffer, renderBuffer.indexBuffer,
                    1, &region);
    vkEndCommandBuffer(transferCommandBuffer);

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    if (vkQueueSubmit(mContext->getTransferQueue(), 1, &submitInfo, mTransferCompleteFences[imageIndex]) != VK_SUCCESS) {
        throw std::runtime_error("Error, failed to submit transfer command");
    }

    mTemporaryStaticBuffers[imageIndex] = renderBuffer;
}