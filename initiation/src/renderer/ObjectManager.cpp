#include "renderer/ObjectManager.hpp"

#include "vulkan/BufferHelper.hpp"

void ObjectManager::create(VulkanContext& context) {
    mContext = &context;
}

void ObjectManager::destroy() {

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

    if (mFirstAllocation) {
        if (mObjects.empty()) return;
        mFirstAllocation = false;

        uint32_t vertexBufferSize{0}, indexBufferSize{0};
        for (Object* o : mObjects) {
            vertexBufferSize += o->getVertexCount();
            indexBufferSize += o->getIndexCount();
        }

        BufferHelper::createBuffer(*mContext,
                                   vertexBufferSize * sizeof(Vertex),
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                   mVertexBuffer);
        BufferHelper::createBuffer(*mContext,
                                   indexBufferSize * sizeof(uint16_t),
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                   mVertexBuffer);
        
        std::vector<Vertex> hostVertexBuffer(vertexBufferSize);
        std::vector<Vertex> hostIndexBuffer(indexBufferSize);

        uint32_t indexOffset{0};
        for (Object* o : mObjects) {
            
        }
    }
}