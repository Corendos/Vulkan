#include "renderer/MeshManager.hpp"

#include "vulkan/BufferHelper.hpp"

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
    if (mRenderData.vertexBufferSize != 0)
        mContext->getMemoryManager().freeBuffer(mRenderData.vertexBuffer);
    if (mRenderData.indexBuffer != 0)
        mContext->getMemoryManager().freeBuffer(mRenderData.indexBuffer);
    mContext->getMemoryManager().freeBuffer(mRenderData.modelTransformBuffer);
}

void MeshManager::addMesh(Mesh& mesh) {
    assert(mMeshes.size() < MaximumMeshCount);
    mMeshes.push_back(&mesh);

    auto descriptorIt = std::find_if(mRenderData.descriptorSetInfos.begin(), mRenderData.descriptorSetInfos.end(),
                                     [](const DescriptorSetInfo& info) { return info.free; });
    assert(descriptorIt != mRenderData.descriptorSetInfos.end());
    descriptorIt->free = false;
    mRenderData.descriptorSetBinding[&mesh] = descriptorIt->descriptorSet;
    updateStaticBuffers();
    updateDescriptorSet(mesh, descriptorIt->descriptorSet);
}

void MeshManager::removeMesh(Mesh& mesh) {
    auto meshIt = std::find(mMeshes.begin(), mMeshes.end(), &mesh);
    assert(meshIt != mMeshes.end());
    mMeshes.erase(meshIt);

    VkDescriptorSet descriptorSet = mRenderData.descriptorSetBinding[&mesh];
    auto descriptorIt = std::find_if(mRenderData.descriptorSetInfos.begin(), mRenderData.descriptorSetInfos.end(),
                                     [&descriptorSet](const DescriptorSetInfo& info) { return info.descriptorSet == descriptorSet; });
    descriptorIt->free = true;
    mRenderData.descriptorSetBinding.erase(&mesh);
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
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
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

    for (size_t i{0};i < MaximumMeshCount;++i) {
        mRenderData.descriptorSetInfos[i].descriptorSet = descriptors[i];
    }
}

void MeshManager::updateStaticBuffers() {
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
    if (mRenderData.vertexBufferSize != 0)
        mContext->getMemoryManager().freeBuffer(mRenderData.vertexBuffer);
    if (mRenderData.indexBufferSize != 0)
        mContext->getMemoryManager().freeBuffer(mRenderData.indexBuffer);
    
    /* Allocate the device local buffers */
    BufferHelper::createBuffer(
        *mContext, vertexBufferSizeInBytes,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mRenderData.vertexBuffer,
        "MeshRenderer::vertexBuffer");
    BufferHelper::createBuffer(
        *mContext, indexBufferSizeInBytes,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mRenderData.indexBuffer,
        "MeshRenderer::indexBuffer");
    
    /* Allocate the staging buffers */
    VkBuffer stagingVertexBuffer, stagingIndexBuffer;
    BufferHelper::createBuffer(
        *mContext, vertexBufferSizeInBytes,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingVertexBuffer,
        "MeshRenderer::stagingVertexBuffer");
    BufferHelper::createBuffer(
        *mContext, indexBufferSizeInBytes,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingIndexBuffer,
        "MeshRenderer::stagingIndexBuffer");
    
    /* Compute the local buffers */
    std::vector<Vertex> localVertexBuffer(vertexBufferSize);
    std::vector<uint32_t> localIndexBuffer(indexBufferSize);

    auto vertexIterator = localVertexBuffer.begin();
    auto indexIterator = localIndexBuffer.begin();
    uint32_t indexOffset{0};
    for (Mesh* mesh : mMeshes) {
        vertexIterator = std::copy(mesh->getVertices().begin(), mesh->getVertices().begin(),
                                   vertexIterator);

        indexIterator = std::transform(
            mesh->getIndices().begin(), mesh->getIndices().begin(),
            indexIterator, [indexOffset](const uint32_t& i) { return i + indexOffset; });
        indexOffset += mesh->getVertices().size();
    }

    /* Copy the buffers */
    void* data;
    mContext->getMemoryManager().mapMemory(stagingVertexBuffer, vertexBufferSizeInBytes, &data);
    memcpy(data, localVertexBuffer.data(), vertexBufferSizeInBytes);
    mContext->getMemoryManager().unmapMemory(stagingVertexBuffer);

    mContext->getMemoryManager().mapMemory(stagingIndexBuffer, indexBufferSizeInBytes, &data);
    memcpy(data, localIndexBuffer.data(), indexBufferSizeInBytes);
    mContext->getMemoryManager().unmapMemory(stagingIndexBuffer);

    BufferHelper::copyBuffer(
        *mContext, mContext->getTransferCommandPool(),
        mContext->getTransferQueue(), stagingVertexBuffer,
        mRenderData.vertexBuffer, vertexBufferSizeInBytes);
    BufferHelper::copyBuffer(
        *mContext, mContext->getTransferCommandPool(),
        mContext->getTransferQueue(), stagingIndexBuffer,
        mRenderData.indexBuffer, indexBufferSizeInBytes);

    /* Free the staging buffers */
    mContext->getMemoryManager().freeBuffer(stagingVertexBuffer);
    mContext->getMemoryManager().freeBuffer(stagingIndexBuffer);

    mRenderData.vertexBufferSize = vertexBufferSize;
    mRenderData.indexBufferSize = indexBufferSize;
}

void MeshManager::updateUniformBuffer() {
    uint32_t size{0}, offset{0};
    if (sizeof(glm::mat4) < mContext->getLimits().minUniformBufferOffsetAlignment) {
        size = mContext->getLimits().minUniformBufferOffsetAlignment * mMeshes.size();
        offset = mContext->getLimits().minUniformBufferOffsetAlignment;
    } else {
        size = sizeof(glm::mat4) * mMeshes.size();
        offset = sizeof(glm::mat4);
    }

    void* data;
    mContext->getMemoryManager().mapMemory(mRenderData.modelTransformBuffer, size, &data);
    for (Mesh* mesh : mMeshes) {
        glm::mat4 m = mesh->getTransform().getMatrix();
        memcpy(data, &m, sizeof(glm::mat4));
        data = (uint8_t*)data + offset;
    }
    mContext->getMemoryManager().unmapMemory(mRenderData.modelTransformBuffer);
}

void MeshManager::updateDescriptorSet(Mesh& mesh, VkDescriptorSet& descriptorSet) {
    VkDescriptorImageInfo info{};
    info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    info.imageView = mesh.getTexture().getImageView().getHandler();
    info.sampler = mesh.getTexture().getSampler().getHandler();

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.dstBinding = 0;
    write.dstSet = descriptorSet;
    write.pImageInfo = &info;

    vkUpdateDescriptorSets(mContext->getDevice(), 1, &write, 0, nullptr);
}