#include "renderer/ObjectManager.hpp"

#include <iostream>

#include "vulkan/BufferHelper.hpp"
#include "vulkan/Commands.hpp"
#include "environment.hpp"

void ObjectManager::create(VulkanContext& context) {
    mContext = &context;
    createDescriptorSetLayout();
    createAndAllocateUniformBuffer();
    allocateDescriptorSets();
}

void ObjectManager::destroy() {
    vkDestroyDescriptorSetLayout(mContext->getDevice(), mDescriptorSetLayout, nullptr);
    mContext->getMemoryManager().freeBuffer(mVertexBuffer);
    mContext->getMemoryManager().freeBuffer(mIndexBuffer);
    mContext->getMemoryManager().freeBuffer(mModelMatrixBuffer);
}

void ObjectManager::addObject(Object& object) {
    /* Find a free descriptor set to associate to the object */
    auto it = std::find_if(mDescriptorSetHandlers.begin(), mDescriptorSetHandlers.end(),
                           [](const DescriptorSetHandler& h) { return !h.used; });
    if (it == mDescriptorSetHandlers.end()) {
        throw std::runtime_error("unable to find a free descriptor set");
    }

    /* Bind it to the object */
    object.setDescriptorSet(it->descriptorSet);
    it->used = true;

    /* Update the descriptor */
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = object.getTexture().getImageView().getHandler();
    imageInfo.sampler = object.getTexture().getSampler().getHandler();

    VkWriteDescriptorSet descriptorSetUpdate = {};
    descriptorSetUpdate.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorSetUpdate.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorSetUpdate.descriptorCount = 1;
    descriptorSetUpdate.dstSet = it->descriptorSet;
    descriptorSetUpdate.dstBinding = 0;
    descriptorSetUpdate.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(mContext->getDevice(),
                           1, &descriptorSetUpdate,
                           0, nullptr);

    /* Add the object to the list of rendered objects */
    mObjects.push_back(&object);
    mObjectsCount++;
}

void ObjectManager::removeObject(Object& object) {
    /* Find the object */
    auto objectIt = std::find(mObjects.begin(), mObjects.end(), &object);

    /* Find the associated descriptor set */
    auto descriptorIt = std::find_if(
        mDescriptorSetHandlers.begin(), mDescriptorSetHandlers.end(),
        [&](const DescriptorSetHandler& h) {
            return h.descriptorSet == (*objectIt)->getDescriptorSet();
        });

    /* Set the descriptor set as free */
    descriptorIt->used = false;
    
    /* Erase the object from the list of rendered objects */
    mObjects.erase(objectIt);
    mObjectsCount++;
}

VkDescriptorSetLayout ObjectManager::getDescriptorSetLayout() const {
    return mDescriptorSetLayout;
}

std::vector<Object*>& ObjectManager::getObjects() {
    return mObjects;
}

void ObjectManager::render(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) {
    uint32_t offset{0};
    VkDeviceSize size{0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mVertexBuffer, &size);
    vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

    for (Object* o : mObjects) {
        VkDescriptorSet descriptorSet = o->getDescriptorSet();
        uint32_t bufferOffset = mObjectDynamicOffset[o];
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipelineLayout, 0, 1, &descriptorSet, 1, &bufferOffset);
        vkCmdDrawIndexed(commandBuffer, o->getIndexCount(),
                         1, offset, 0, 0);
        offset += o->getIndexCount();
    }
}

void ObjectManager::updateBuffers() {    
    // TODO: handle update when it's not the first allocation
    if (mFirstAllocation) {
        if (mObjects.empty()) return;
        mFirstAllocation = false;

        /* Retrieve the buffer size */
        uint32_t vertexBufferSize{0}, indexBufferSize{0};
        for (Object* o : mObjects) {
            vertexBufferSize += o->getVertexCount();
            indexBufferSize += o->getIndexCount();
        }

        uint32_t vertexBufferSizeInBytes = vertexBufferSize * sizeof(Vertex);
        uint32_t indexBufferSizeInBytes = indexBufferSize * sizeof(uint32_t);

        /* If it's the first update, allocate without freeing the buffers */
        BufferHelper::createBuffer(*mContext,
                                   vertexBufferSizeInBytes,
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                   mVertexBuffer);
        BufferHelper::createBuffer(*mContext,
                                   indexBufferSizeInBytes,
                                   VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                   mIndexBuffer);
        
        /* Copy and transform the data locally */
        std::vector<Vertex> hostVertexBuffer(vertexBufferSize);
        std::vector<uint32_t> hostIndexBuffer(indexBufferSize);

        uint32_t indexOffset{0};
        uint32_t dynamicOffset{0};
        auto vertexIterator = hostVertexBuffer.begin();
        auto indexIterator = hostIndexBuffer.begin();
        for (Object* o : mObjects) {
            vertexIterator = std::copy(
                o->getVertices().begin(), o->getVertices().end(),
                vertexIterator
            );
            indexIterator = std::transform(
                o->getIndices().begin(), o->getIndices().end(),
                indexIterator, [&indexOffset](const uint32_t& index) { return index + indexOffset; }
            );
            indexOffset += o->getVertexCount();
            glm::mat4 m = o->getTransform().getMatrix();
            mObjectDynamicOffset.insert(std::make_pair(o, dynamicOffset));
            if (sizeof(glm::mat4) < mContext->getLimits().minUniformBufferOffsetAlignment) {
                dynamicOffset += mContext->getLimits().minUniformBufferOffsetAlignment;
            } else {
                dynamicOffset += sizeof(glm::mat4);
            }
        }

        /* Allocate for the staging buffers */
        VkBuffer stagingVertexBuffer;
        VkBuffer stagingIndexBuffer;

        BufferHelper::createBuffer(*mContext,
                                   vertexBufferSizeInBytes,
                                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                   stagingVertexBuffer);
        BufferHelper::createBuffer(*mContext,
                                   indexBufferSizeInBytes,
                                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                   stagingIndexBuffer);

        /* Copy the data in the staging buffers */
        void* data;
        mContext->getMemoryManager().mapMemory(stagingVertexBuffer, vertexBufferSizeInBytes, &data);
        memcpy(data, hostVertexBuffer.data(), vertexBufferSizeInBytes);
        mContext->getMemoryManager().unmapMemory(stagingVertexBuffer);

        mContext->getMemoryManager().mapMemory(stagingIndexBuffer, indexBufferSizeInBytes, &data);
        memcpy(data, hostIndexBuffer.data(), indexBufferSizeInBytes);
        mContext->getMemoryManager().unmapMemory(stagingIndexBuffer);

        /* Transfer the data to the device */
        VkCommandBuffer transferCommandBuffer = Commands::beginSingleTime(mContext->getDevice(), mContext->getTransferCommandPool());
        BufferHelper::copyBuffer(*mContext,
                                 mContext->getTransferCommandPool(),
                                 mContext->getTransferQueue(),
                                 stagingVertexBuffer,
                                 mVertexBuffer,
                                 vertexBufferSizeInBytes);
        BufferHelper::copyBuffer(*mContext,
                                 mContext->getTransferCommandPool(),
                                 mContext->getTransferQueue(),
                                 stagingIndexBuffer,
                                 mIndexBuffer,
                                 indexBufferSizeInBytes);
        Commands::endSingleTime(mContext->getDevice(), mContext->getTransferCommandPool(), transferCommandBuffer, mContext->getTransferQueue());

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = mModelMatrixBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(glm::mat4);

        uint32_t size, offset;
        if (sizeof(glm::mat4) < mContext->getLimits().minUniformBufferOffsetAlignment) {
            size = mContext->getLimits().minUniformBufferOffsetAlignment * mObjectsCount;
            offset = mContext->getLimits().minUniformBufferOffsetAlignment;
        } else {
            size = sizeof(glm::mat4) * mObjectsCount;
            offset = sizeof(glm::mat4);
        }

        std::vector<VkWriteDescriptorSet> descriptorSetUpdates(mObjects.size());

        mContext->getMemoryManager().mapMemory(mModelMatrixBuffer, size, &data);
        for (size_t i{0}; i < mObjectsCount;i++) {
            Object* o = mObjects[i];
            descriptorSetUpdates[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorSetUpdates[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            descriptorSetUpdates[i].descriptorCount = 1;
            descriptorSetUpdates[i].dstSet = o->getDescriptorSet();
            descriptorSetUpdates[i].dstBinding = 1;
            descriptorSetUpdates[i].pBufferInfo = &bufferInfo;
            glm::mat4 mat = o->getTransform().getMatrix();
            memcpy(data, &mat, sizeof(glm::mat4));
            data = data + offset;
        }
        mContext->getMemoryManager().unmapMemory(mModelMatrixBuffer);

        vkUpdateDescriptorSets(mContext->getDevice(),
                               static_cast<uint32_t>(descriptorSetUpdates.size()),
                               descriptorSetUpdates.data(),
                               0, nullptr);

        /* Free the staging buffers */
        mContext->getMemoryManager().freeBuffer(stagingVertexBuffer);
        mContext->getMemoryManager().freeBuffer(stagingIndexBuffer);
    }
}

void ObjectManager::updateUniformBuffer() {
    uint32_t size, offset;
    if (sizeof(glm::mat4) < mContext->getLimits().minUniformBufferOffsetAlignment) {
        size = mContext->getLimits().minUniformBufferOffsetAlignment * mObjectsCount;
        offset = mContext->getLimits().minUniformBufferOffsetAlignment;
    } else {
        size = sizeof(glm::mat4) * mObjectsCount;
        offset = sizeof(glm::mat4);
    }
    void* data;
    mContext->getMemoryManager().mapMemory(mModelMatrixBuffer, size, &data);
    for (size_t i{0}; i < mObjectsCount;i++) {
        glm::mat4 mat = mObjects[i]->getTransform().getMatrix();
        memcpy(data, &mat, sizeof(glm::mat4));
        data = data + offset;
    }
    mContext->getMemoryManager().unmapMemory(mModelMatrixBuffer);
}

void ObjectManager::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding bindings[2] = {};
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = 2;
    createInfo.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(mContext->getDevice(), &createInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout");
    }
}

void ObjectManager::createAndAllocateUniformBuffer() {
    uint32_t size;

    if (sizeof(glm::mat4) > mContext->getLimits().minUniformBufferOffsetAlignment) {
        size = mMaxObjectCount * sizeof(glm::mat4);
    } else {
        size = mMaxObjectCount * mContext->getLimits().minUniformBufferOffsetAlignment;
    }

    BufferHelper::createBuffer(*mContext,
                               size,
                               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                               mModelMatrixBuffer);
}

void ObjectManager::allocateDescriptorSets() {
    /* Create a temporary array to store the descriptor handlers */
    std::vector<VkDescriptorSet> descriptorSets(mMaxObjectCount);

    /* Allocate the descriptors */
    std::vector<VkDescriptorSetLayout> layouts(mMaxObjectCount, mDescriptorSetLayout);
    VkDescriptorSetAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    info.descriptorPool = mContext->getDescriptorPool().getHandler();
    info.descriptorSetCount = mMaxObjectCount;
    info.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(mContext->getDevice(), &info, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets");
    }

    /* Copy them in the descriptor smart handler array */
    mDescriptorSetHandlers.resize(mMaxObjectCount);

    for (size_t i{0};i < descriptorSets.size();++i) {
        mDescriptorSetHandlers[i].descriptorSet = descriptorSets[i];
    }
}