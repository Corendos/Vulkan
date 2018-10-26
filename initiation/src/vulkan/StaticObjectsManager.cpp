#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "vulkan/StaticObjectsManager.hpp"
#include "vulkan/Vulkan.hpp"
#include "memory/MemoryManager.hpp"
#include "vulkan/BufferHelper.hpp"
#include "vulkan/UniformBufferObject.hpp"
#include "utils.hpp"

void StaticObjectsManager::addStaticObject(StaticObject& staticObject) {
    mStaticObjects.push_back(staticObject);
    mVertices.insert(mVertices.end(), staticObject.getVertices().begin(), staticObject.getVertices().end());
    mIndices.insert(mIndices.end(), staticObject.getIndices().begin(), staticObject.getIndices().end());
}

void StaticObjectsManager::create(Vulkan& vulkan) {
    mVulkan = &vulkan;
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffer();
}

void StaticObjectsManager::destroy() {
    mVulkan->getMemoryManager().freeBuffer(mVertexBuffer);
    mVulkan->getMemoryManager().freeBuffer(mIndexBuffer);
    mVulkan->getMemoryManager().freeBuffer(mUniformBuffer);
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


void StaticObjectsManager::createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(Vertex) * mVertices.size();

    VkBuffer stagingBuffer;

    BufferHelper::createBuffer(mVulkan->getMemoryManager(), mVulkan->getDevice(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer);
    
    void* data;
    mVulkan->getMemoryManager().mapMemory(stagingBuffer, bufferSize, &data);
    memcpy(data, mVertices.data(), (size_t)bufferSize);
    mVulkan->getMemoryManager().unmapMemory(stagingBuffer);

    BufferHelper::createBuffer(mVulkan->getMemoryManager(), mVulkan->getDevice(), bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mVertexBuffer);

    BufferHelper::copyBuffer(
        mVulkan->getMemoryManager(),
        mVulkan->getDevice(),
        mVulkan->getCommandPool(),
        mVulkan->getGraphicsQueue(), stagingBuffer, mVertexBuffer, bufferSize);

    mVulkan->getMemoryManager().freeBuffer(stagingBuffer);
}

void StaticObjectsManager::createIndexBuffer() {
    VkDeviceSize bufferSize = sizeof(Vertex) * mIndices.size();

    VkBuffer stagingBuffer;

    BufferHelper::createBuffer(mVulkan->getMemoryManager(), mVulkan->getDevice(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer);
    
    void* data;
    mVulkan->getMemoryManager().mapMemory(stagingBuffer, bufferSize, &data);
    memcpy(data, mIndices.data(), (size_t)bufferSize);
    mVulkan->getMemoryManager().unmapMemory(stagingBuffer);

    BufferHelper::createBuffer(mVulkan->getMemoryManager(), mVulkan->getDevice(), bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mIndexBuffer);

    BufferHelper::copyBuffer(
        mVulkan->getMemoryManager(),
        mVulkan->getDevice(),
        mVulkan->getCommandPool(),
        mVulkan->getGraphicsQueue(), stagingBuffer, mIndexBuffer, bufferSize);

    mVulkan->getMemoryManager().freeBuffer(stagingBuffer);    
}

void StaticObjectsManager::createUniformBuffer() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    BufferHelper::createBuffer(mVulkan->getMemoryManager(), mVulkan->getDevice(), bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mUniformBuffer);

    UniformBufferObject ubo{};
    ubo.model = glm::mat4(1.0f);
    ubo.view = glm::lookAt(
        glm::vec3(2.0f, 2.0f, 2.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::vulkanPerspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 10.0f);

    void* data;
    mVulkan->getMemoryManager().mapMemory(mUniformBuffer, sizeof(ubo), &data);
    memcpy(data, &ubo, sizeof(ubo));
    mVulkan->getMemoryManager().unmapMemory(mUniformBuffer);
}