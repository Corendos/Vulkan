#include "vulkan/StaticObjectsManager.hpp"

void StaticObjectsManager::addStaticObject(StaticObject& staticObject) {
    mStaticObjects.push_back(staticObject);
    mVertices.insert(mVertices.end(), staticObject.getVertices().begin(), staticObject.getVertices().end());
    mIndices.insert(mIndices.end(), staticObject.getIndices().begin(), staticObject.getIndices().end());
}

void StaticObjectsManager::create(VkDevice device, MemoryManager& manager, CommandPool& commandPool, VkQueue queue) {
    mMemoryManager = &manager;
    createVertexBuffer(device, commandPool, queue);
    createIndexBuffer(device, commandPool, queue);
    createUniformBuffer(device);
}

void StaticObjectsManager::destroy(VkDevice device) {
    mMemoryManager->freeBuffer(mVertexBuffer);
    mMemoryManager->freeBuffer(mIndexBuffer);
    mMemoryManager->freeBuffer(mUniformBuffer);
}

VkBuffer StaticObjectsManager::getUniformBuffer() const {
    return mUniformBuffer;
}

VkBuffer StaticObjectsManager::getVertexBuffer() const {
    return mVertexBuffer;
}

VkBuffer StaticObjectsManager::getindexBuffer() const {
    return mIndexBuffer;
}


void StaticObjectsManager::createVertexBuffer(VkDevice device, CommandPool& commandPool, VkQueue queue) {
    VkDeviceSize bufferSize = sizeof(Vertex) * mVertices.size();

    VkBuffer stagingBuffer;

    BufferHelper::createBuffer(*mMemoryManager, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer);
    
    void* data;
    mMemoryManager->mapMemory(stagingBuffer, bufferSize, &data);
    memcpy(data, mVertices.data(), (size_t)bufferSize);
    mMemoryManager->unmapMemory(stagingBuffer);

    BufferHelper::createBuffer(*mMemoryManager, device, bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mVertexBuffer);

    BufferHelper::copyBuffer(*mMemoryManager, device, commandPool, queue, stagingBuffer, mVertexBuffer, bufferSize);

    mMemoryManager->freeBuffer(stagingBuffer);
}

void StaticObjectsManager::createIndexBuffer(VkDevice device, CommandPool& commandPool, VkQueue queue) {
    VkDeviceSize bufferSize = sizeof(Vertex) * mIndices.size();

    VkBuffer stagingBuffer;

    BufferHelper::createBuffer(*mMemoryManager, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer);
    
    void* data;
    mMemoryManager->mapMemory(stagingBuffer, bufferSize, &data);
    memcpy(data, mIndices.data(), (size_t)bufferSize);
    mMemoryManager->unmapMemory(stagingBuffer);

    BufferHelper::createBuffer(*mMemoryManager, device, bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mIndexBuffer);

    BufferHelper::copyBuffer(*mMemoryManager, device, commandPool, queue, stagingBuffer, mIndexBuffer, bufferSize);

    mMemoryManager->freeBuffer(stagingBuffer);    
}

void StaticObjectsManager::createUniformBuffer(VkDevice device) {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    BufferHelper::createBuffer(*mMemoryManager, device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mUniformBuffer);

    UniformBufferObject ubo{};
    ubo.model = glm::mat4(1.0f);
    ubo.view = glm::lookAt(
        glm::vec3(2.0f, 2.0f, 2.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::vulkanPerspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 10.0f);

    void* data;
    mMemoryManager->mapMemory(mUniformBuffer, sizeof(ubo), &data);
    memcpy(data, &ubo, sizeof(ubo));
    mMemoryManager->unmapMemory(mUniformBuffer);
}