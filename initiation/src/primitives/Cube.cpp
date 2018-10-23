#include "primitives/Cube.hpp"

Cube::Cube(float size) {
    mSize = size;
    std::vector<Vertex> vertices = {
        {{size, size, size}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-size, size, size}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{-size, -size, size}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{size, -size, size}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{size, size, -size}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{-size, size, -size}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-size, -size, -size}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{size, -size, -size}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}
    };
    std::copy(vertices.begin(), vertices.end(), mVertices.begin());
}

void Cube::create(MemoryManager& manager,
                  VkDevice device,
                  CommandPool& commandPool,
                  VkQueue queue,
                  SwapChain& swapChain,
                  VkDescriptorPool descriptorPool,
                  VkDescriptorSetLayout layout,
                  VkImageView imageView,
                  VkSampler sampler) {
    VkDeviceSize bufferSize = sizeof(mVertices[0]) * mVertices.size();

    VkBuffer stagingBuffer;
    BufferHelper::createBuffer(manager, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer);

    void* data;
    manager.mapMemory(stagingBuffer, bufferSize, &data);
    memcpy(data, mVertices.data(), (size_t)bufferSize);
    manager.unmapMemory(stagingBuffer);

    BufferHelper::createBuffer(manager, device, bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        mVertexBuffer);
    
    BufferHelper::copyBuffer(manager, device, commandPool, queue, stagingBuffer, mVertexBuffer, bufferSize);

    manager.freeBuffer(stagingBuffer);


    VkDeviceSize size = sizeof(mIndices[0]) * mIndices.size();

    BufferHelper::createBuffer(manager, device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer);

    manager.mapMemory(stagingBuffer, size, &data);
    memcpy(data, mIndices.data(), (size_t)size);
    manager.unmapMemory(stagingBuffer);

    BufferHelper::createBuffer(manager, device, size,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        mIndicesBuffer);

    BufferHelper::copyBuffer(manager, device, commandPool, queue, stagingBuffer, mIndicesBuffer, size);

    manager.freeBuffer(stagingBuffer);

    bufferSize = sizeof(UniformBufferObject);

    BufferHelper::createBuffer(manager, device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            mUniformBuffer);

    UniformBufferObject ubo{};
    ubo.model = glm::mat4(1.0f);
    ubo.view = glm::lookAt(
        glm::vec3(2.0f, 2.0f, 2.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::vulkanPerspective(glm::radians(45.0f), swapChain.getExtent().width / (float)swapChain.getExtent().height, 0.1f, 10.0f);

    manager.mapMemory(mUniformBuffer, sizeof(ubo), &data);
    memcpy(data, &ubo, sizeof(ubo));
    manager.unmapMemory(mUniformBuffer);

    createDescriptor(device, descriptorPool, layout, imageView, sampler);
}

void Cube::destroy(MemoryManager& memoryManager) {
    memoryManager.freeBuffer(mVertexBuffer);
    memoryManager.freeBuffer(mIndicesBuffer);
    memoryManager.freeBuffer(mUniformBuffer);
}

VkBuffer Cube::getVertexBuffer() {
    return mVertexBuffer;
}

VkBuffer Cube::getIndicesBuffer() {
    return mIndicesBuffer;
}

VkDescriptorSet Cube::getDescriptorSet() {
    return mDescriptor;
}

void Cube::createDescriptor(VkDevice device,
                            VkDescriptorPool descriptorPool,
                            VkDescriptorSetLayout layout,
                            VkImageView imageView,
                            VkSampler sampler) {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    if (vkAllocateDescriptorSets(device, &allocInfo, &mDescriptor) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor sets");
    }

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = mUniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = mDescriptor;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;
    descriptorWrite.pImageInfo = nullptr;
    descriptorWrite.pTexelBufferView = nullptr;

    VkWriteDescriptorSet sets[] = {descriptorWrite};

    vkUpdateDescriptorSets(device, 1, sets, 0, nullptr);
}