#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "vulkan/StaticObjectsManager.hpp"
#include "memory/MemoryManager.hpp"
#include "vulkan/BufferHelper.hpp"
#include "vulkan/UniformBufferObject.hpp"
#include "renderer/Renderer.hpp"
#include "utils.hpp"

void StaticObjectsManager::addStaticObject(StaticObject staticObject) {
    StaticObjectInfo info{};
    info.object = staticObject;
    info.indicesOffset = mIndices.size();
    info.indiceCount = staticObject.getIndices().size();

    mStaticObjectsInfo.push_back(info);
    std::vector<uint16_t> indicesCopy(staticObject.getIndices().size());
    std::transform(staticObject.getIndices().begin(), staticObject.getIndices().end(),
        indicesCopy.begin(), [this](const uint16_t& value) { return value + mVertices.size(); });
    mVertices.insert(mVertices.end(), staticObject.getVertices().begin(), staticObject.getVertices().end());
    mIndices.insert(mIndices.end(), indicesCopy.begin(), indicesCopy.end());
}

void StaticObjectsManager::create(VulkanContext& context, Renderer& renderer) {
    mRenderer = &renderer;
    mContext = &context;
    createVertexBuffer();
    createIndexBuffer();
    createDescriptorSets(mContext->getDevice(),
                         mRenderer->getDescriptorPool(),
                         mRenderer->getDescriptorSetLayout());
}

void StaticObjectsManager::destroy() {
    mContext->getMemoryManager().freeBuffer(mVertexBuffer);
    mContext->getMemoryManager().freeBuffer(mIndexBuffer);
}

VkBuffer StaticObjectsManager::getVertexBuffer() const {
    return mVertexBuffer;
}

VkBuffer StaticObjectsManager::getIndexBuffer() const {
    return mIndexBuffer;
}

uint32_t StaticObjectsManager::getIndiceCount() const {
    return mIndices.size();
}

std::vector<StaticObjectInfo>& StaticObjectsManager::getObjectInfos() {
    return mStaticObjectsInfo;
}

StaticObjectInfo& StaticObjectsManager::getObjectInfo(uint32_t index) {
    return mStaticObjectsInfo[index];
}

std::vector<VkDescriptorSet>& StaticObjectsManager::getDescriptors() {
    return mDescriptorSets;
}

VkDescriptorSet StaticObjectsManager::getDescriptor(uint32_t index) {
    return mDescriptorSets[index];
}

void StaticObjectsManager::createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(Vertex) * mVertices.size();

    VkBuffer stagingBuffer;

    BufferHelper::createBuffer(mContext->getMemoryManager(), mContext->getDevice(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer);
    
    void* data;
    mContext->getMemoryManager().mapMemory(stagingBuffer, bufferSize, &data);
    memcpy(data, mVertices.data(), (size_t)bufferSize);
    mContext->getMemoryManager().unmapMemory(stagingBuffer);

    BufferHelper::createBuffer(mContext->getMemoryManager(), mContext->getDevice(), bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mVertexBuffer);

    BufferHelper::copyBuffer(
        mContext->getMemoryManager(),
        mContext->getDevice(),
        mRenderer->getCommandPool(),
        mContext->getGraphicsQueue(), stagingBuffer, mVertexBuffer, bufferSize);

    mContext->getMemoryManager().freeBuffer(stagingBuffer);
}

void StaticObjectsManager::createIndexBuffer() {
    VkDeviceSize bufferSize = sizeof(Vertex) * mIndices.size();

    VkBuffer stagingBuffer;

    BufferHelper::createBuffer(mContext->getMemoryManager(), mContext->getDevice(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer);
    
    void* data;
    mContext->getMemoryManager().mapMemory(stagingBuffer, bufferSize, &data);
    memcpy(data, mIndices.data(), (size_t)bufferSize);
    mContext->getMemoryManager().unmapMemory(stagingBuffer);

    BufferHelper::createBuffer(mContext->getMemoryManager(), mContext->getDevice(), bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mIndexBuffer);

    BufferHelper::copyBuffer(
        mContext->getMemoryManager(),
        mContext->getDevice(),
        mRenderer->getCommandPool(),
        mContext->getGraphicsQueue(), stagingBuffer, mIndexBuffer, bufferSize);

    mContext->getMemoryManager().freeBuffer(stagingBuffer);    
}

void StaticObjectsManager::createDescriptorSets(VkDevice device,
                                                VkDescriptorPool descriptorPool,
                                                VkDescriptorSetLayout descriptorSetLayout) {
    mDescriptorSets.resize(mStaticObjectsInfo.size());

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    for (size_t i{0};i < mDescriptorSets.size();++i) {
        if (vkAllocateDescriptorSets(device, &allocInfo, &mDescriptorSets[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate descriptor sets");
        }
    }

    for (size_t i = 0;i < mDescriptorSets.size();++i) {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = mStaticObjectsInfo[i].object.getTexture().getViewHandler();
        imageInfo.sampler = mStaticObjectsInfo[i].object.getTexture().getSamplerHandler();

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = mDescriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = nullptr;
        descriptorWrite.pImageInfo = &imageInfo;
        descriptorWrite.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }
}
