#include "renderer/ObjectManager.hpp"

#include "vulkan/BufferHelper.hpp"
#include "vulkan/Commands.hpp"

void ObjectManager::create(VulkanContext& context) {
    mContext = &context;
}

void ObjectManager::destroy() {
    mContext->getMemoryManager().freeBuffer(mVertexBuffer);
    mContext->getMemoryManager().freeBuffer(mIndexBuffer);
}

void ObjectManager::addObject(Object& object) {
    mObjects.push_back(&object);
    mUpdateNeeded = true;
}

void ObjectManager::removeObject(Object& object) {
    auto it = std::find(mObjects.begin(), mObjects.end(), &object);
    mObjects.erase(it);
    mUpdateNeeded = true;
}

void ObjectManager::update() {
    if (!mUpdateNeeded) return;
    
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
        uint32_t indexBufferSizeInBytes = indexBufferSize * sizeof(uint16_t);

        /* If it's the first update, allocate without freeing the buffers */
        BufferHelper::createBuffer(*mContext,
                                   vertexBufferSizeInBytes,
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                   mVertexBuffer);
        BufferHelper::createBuffer(*mContext,
                                   indexBufferSizeInBytes,
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                   mIndexBuffer);
        
        /* Copy and transform the data locally */
        std::vector<Vertex> hostVertexBuffer(vertexBufferSize);
        std::vector<uint16_t> hostIndexBuffer(indexBufferSize);

        uint32_t indexOffset{0};
        auto vertexIterator = hostVertexBuffer.begin();
        auto indexIterator = hostIndexBuffer.begin();
        for (Object* o : mObjects) {
            vertexIterator = std::copy(
                o->getVertices().begin(), o->getVertices().end(),
                vertexIterator
            );
            indexIterator = std::transform(
                o->getIndices().begin(), o->getIndices().end(),
                indexIterator, [&indexOffset](const uint16_t& index) { return index + indexOffset; }
            );
            indexOffset += o->getVertexCount();
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
        memcpy(data, hostIndexBuffer.data(), indexBufferSize);
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

        /* Free the staging buffers */
        mContext->getMemoryManager().freeBuffer(stagingVertexBuffer);
        mContext->getMemoryManager().freeBuffer(stagingIndexBuffer);
    }
}