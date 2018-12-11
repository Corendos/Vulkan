#include "renderer/mesh/MeshManager.hpp"

#include "vulkan/buffer/BufferHelper.hpp"

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
}

void MeshManager::addMesh(Mesh& mesh) {
    assert(mMeshes.size() < MaximumMeshCount);
    mMeshes.push_back(&mesh);

    auto meshDataIt = std::find_if(mRenderData.meshDataPool.begin(), mRenderData.meshDataPool.end(),
                                     [](const MeshData& data) { return data.free; });
    assert(meshDataIt != mRenderData.meshDataPool.end());
    meshDataIt->free = false;
    mRenderData.meshDataBinding[&mesh] = &(*meshDataIt);
    updateStagingBuffers();
    updateDescriptorSet(mesh, *meshDataIt);
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
    updateStagingBuffers();
}

void MeshManager::setImageCount(uint32_t count) {
    mRenderData.renderBuffers.resize(count);
}

bool MeshManager::update(uint32_t imageIndex) {
    updateUniformBuffer();
    return updateStaticBuffers(imageIndex);
}

void MeshManager::render(VkCommandBuffer commandBuffer, VkPipelineLayout layout, uint32_t imageIndex) {
    uint32_t offset{0};
    VkDeviceSize vertexOffset{0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mRenderData.renderBuffers[imageIndex].vertexBuffer, &vertexOffset);
    vkCmdBindIndexBuffer(commandBuffer, mRenderData.renderBuffers[imageIndex].indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    for (Mesh* mesh : mMeshes) {
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            layout, 0, 1, &mRenderData.meshDataBinding[mesh]->descriptorSet,
            0, nullptr);
        vkCmdDrawIndexed(commandBuffer, mesh->getIndices().size(), 1, offset, 0, 0);
        offset += mesh->getIndices().size();
    }
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

bool MeshManager::updateStaticBuffers(uint32_t imageIndex) {
    if (!mRenderData.renderBuffers[imageIndex].needUpdate) return false;

    if (mRenderData.renderBuffers[imageIndex].vertexBufferSizeInBytes != 0)
        mContext->getMemoryManager().freeBuffer(mRenderData.renderBuffers[imageIndex].vertexBuffer);
    if (mRenderData.renderBuffers[imageIndex].indexBufferSizeInBytes != 0)
        mContext->getMemoryManager().freeBuffer(mRenderData.renderBuffers[imageIndex].indexBuffer);
    
    BufferHelper::createBuffer(
        *mContext, mRenderData.stagingBuffers.vertexBufferSizeInBytes,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        mRenderData.renderBuffers[imageIndex].vertexBuffer,
        "MeshRenderer::vertexBuffer");
    BufferHelper::createBuffer(
        *mContext, mRenderData.stagingBuffers.indexBufferSizeInBytes,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        mRenderData.renderBuffers[imageIndex].indexBuffer,
        "MeshRenderer::indexBuffer");

    BufferHelper::copyBuffer(
        *mContext, mContext->getTransferCommandPool(),
        mContext->getTransferQueue(), mRenderData.stagingBuffers.vertexBuffer,
        mRenderData.renderBuffers[imageIndex].vertexBuffer,
        mRenderData.stagingBuffers.vertexBufferSizeInBytes);
    BufferHelper::copyBuffer(
        *mContext, mContext->getTransferCommandPool(),
        mContext->getTransferQueue(), mRenderData.stagingBuffers.indexBuffer,
        mRenderData.renderBuffers[imageIndex].indexBuffer,
        mRenderData.stagingBuffers.indexBufferSizeInBytes);

    mRenderData.renderBuffers[imageIndex].vertexBufferSize = mRenderData.stagingBuffers.vertexBufferSize;
    mRenderData.renderBuffers[imageIndex].vertexBufferSizeInBytes = mRenderData.stagingBuffers.vertexBufferSizeInBytes;
    mRenderData.renderBuffers[imageIndex].indexBufferSize = mRenderData.stagingBuffers.indexBufferSize;
    mRenderData.renderBuffers[imageIndex].indexBufferSizeInBytes = mRenderData.stagingBuffers.indexBufferSizeInBytes;

    mRenderData.renderBuffers[imageIndex].needUpdate = false;
    return true;
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
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        mRenderData.stagingBuffers.vertexBuffer,
        "MeshRenderer::stagingVertexBuffer");
    BufferHelper::createBuffer(
        *mContext, indexBufferSizeInBytes,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
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

    mRenderData.stagingBuffers.vertexBufferSize = vertexBufferSize;
    mRenderData.stagingBuffers.vertexBufferSizeInBytes = vertexBufferSizeInBytes;
    mRenderData.stagingBuffers.indexBufferSize = indexBufferSize;
    mRenderData.stagingBuffers.indexBufferSizeInBytes = indexBufferSizeInBytes;

    for (auto& buffers : mRenderData.renderBuffers) {
        buffers.needUpdate = true;
    }
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