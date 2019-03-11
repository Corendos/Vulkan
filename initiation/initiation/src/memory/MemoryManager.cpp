#include <iostream>
#include <fstream>
#include <iomanip>

#include "memory/MemoryManager.hpp"

#include "PrintHelper.hpp"
#include "utils.hpp"

uint32_t MemoryManager::allocationSize = 32 * mega;
uint32_t MemoryManager::pageSize = 4 * kilo;

MemoryManager::MemoryManager(VkPhysicalDevice& physicalDevice, VkDevice& device) :
    mDevice(device), mPhysicalDevice(physicalDevice) {
}

void MemoryManager::init() {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(mPhysicalDevice, &properties);

    if (pageSize < properties.limits.bufferImageGranularity) {
        pageSize = properties.limits.bufferImageGranularity;
    }

    vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &mMemoryProperties);
}

void MemoryManager::printInfo() {
    std::cout << "[Allocated MemoryTypeCount]" << std::endl;
    std::cout << mChunksMap.size() << std::endl << std::endl;
    
    std::cout << "[Chunks Allocation Summary]" << std::endl;
    for (auto& pair : mChunksMap) {
        std::cout << "Memory Type: " << pair.first << std::endl;
        for (uint32_t i{0};i < pair.second.size();++i) {
            Chunk& chunk = pair.second[i];
            std::cout << "\tChunk #" << i << " allocated " << chunk.getTree().getValue().size << " bytes" << std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << "[Buffer Block Summary]" << std::endl;
    for (auto& pair : mBuffersInfo) {
        std::cout << "Buffer " << pair.first
                << "\tMemoryTypeIndex: " << pair.second.memoryTypeIndex << "\n"
                << "\tChunkIndex: " << pair.second.chunkIndex << "\n"
                << "\tBlock size: " << pair.second.block.size << "\n"
                << "\tBlock offset: " << pair.second.block.offset << "\n" << std::endl;
    }

    std::cout << "[Image Block Summary]" << std::endl;
    for (auto& pair : mImagesInfo) {
        std::cout << "Image " << pair.first
                << "\tMemoryTypeIndex: " << pair.second.memoryTypeIndex << "\n"
                << "\tChunkIndex: " << pair.second.chunkIndex << "\n"
                << "\tBlock size: " << pair.second.block.size << "\n"
                << "\tBlock offset: " << pair.second.block.offset << "\n" << std::endl;
    }
}

void MemoryManager::memoryCheckLog() {
    std::ofstream file;
    file.open("./memory.log", std::ios::out | std::ios::trunc);

    for (auto& pair : mChunksMap) {
        file << "Memory Type #" << pair.first << " : " << pair.second.size() << " allocation(s)" << std::endl;
    }

    file.close();
}

void MemoryManager::allocate(uint32_t memoryTypeIndex) {
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    allocInfo.allocationSize = allocationSize;

    VkDeviceMemory memoryAllocation;
    VkResult result = vkAllocateMemory(mDevice, &allocInfo, nullptr, &memoryAllocation); 
    if (result != VK_SUCCESS) {
        // This is temporary, if the allocation is not successful, it could be harmless
        std::cout << "Failed to allocate memory" << std::endl;
    }

    if (mChunksMap.find(memoryTypeIndex) == mChunksMap.end()) {
        mChunksMap[memoryTypeIndex] = std::vector<Chunk>();
    }

    mChunksMap[memoryTypeIndex].push_back(Chunk(memoryAllocation, allocationSize, pageSize));
}

void MemoryManager::cleanup() {
    for (auto& pair : mChunksMap) {
        for (auto& chunk : pair.second) {
            vkFreeMemory(mDevice, chunk.getMemory(), nullptr);
        }
    }
}

void MemoryManager::allocateForBuffer(VkBuffer buffer,
                                      VkMemoryRequirements& memoryRequirements,
                                      VkMemoryPropertyFlags properties,
                                      std::string name) {
    int32_t memoryTypeIndex = findMemoryType(memoryRequirements, properties);

    if (memoryTypeIndex == -1) {
        throw std::runtime_error("Unable to find a suitable memory type");
    }

    if (mChunksMap.find(memoryTypeIndex) == mChunksMap.end()) {
        allocate(memoryTypeIndex);
    }

    for (uint32_t i{0};i < mChunksMap.size();++i) {
        Chunk& chunk = mChunksMap[memoryTypeIndex][i];
        AllocationResult result = chunk.reserve(memoryRequirements.size);
        if (result.found) {
            Block block = result.block;
            //mBuffersInfo[buffer] = {}; // For testing purpose
            mBuffersInfo[buffer].block = block;
            mBuffersInfo[buffer].chunkIndex = i;
            mBuffersInfo[buffer].memoryTypeIndex = memoryTypeIndex;

            vkBindBufferMemory(mDevice, buffer, chunk.getMemory(), block.offset);
            return;
        }
    }

    // If we come to this point, this means we need to allocate a new Chunk of memory because the previous one are full
    allocate(memoryTypeIndex);
    Chunk& chunk = mChunksMap[memoryTypeIndex].back();
    AllocationResult result = chunk.reserve(memoryRequirements.size);
    if (result.found) {
        Block block = result.block;
        //mBuffersInfo[buffer] = {}; // For testing purpose
        mBuffersInfo[buffer].block = block;
        mBuffersInfo[buffer].chunkIndex = mChunksMap[memoryTypeIndex].size() - 1;
        mBuffersInfo[buffer].memoryTypeIndex = memoryTypeIndex;

        vkBindBufferMemory(mDevice, buffer, chunk.getMemory(), block.offset);
        return;
    }

    throw std::runtime_error("Unable to find enough memory");
}

void MemoryManager::allocateForImage(VkImage image,
                                     VkMemoryRequirements& memoryRequirements,
                                     VkMemoryPropertyFlags properties,
                                     std::string name) {
    int32_t memoryTypeIndex = findMemoryType(memoryRequirements, properties);

    if (memoryTypeIndex == -1) {
        throw std::runtime_error("Unable to find a suitable memory type");
    }

    if (mChunksMap.find(memoryTypeIndex) == mChunksMap.end()) {
        allocate(memoryTypeIndex);
    }

    for (uint32_t i{0};i < mChunksMap[memoryTypeIndex].size();++i) {
        Chunk& chunk = mChunksMap[memoryTypeIndex][i];
        AllocationResult result = chunk.reserve(memoryRequirements.size);
        if (result.found) {
            Block block = result.block;
            //mBuffersInfo[buffer] = {}; // For testing purpose
            mImagesInfo[image].block = block;
            mImagesInfo[image].chunkIndex = i;
            mImagesInfo[image].memoryTypeIndex = memoryTypeIndex;

            vkBindImageMemory(mDevice, image, chunk.getMemory(), block.offset);
            return;
        }
    }

    // If we come to this point, this means we need to allocate a new Chunk of memory because the previous one are full
    allocate(memoryTypeIndex);
    Chunk& chunk = mChunksMap[memoryTypeIndex].back();
    AllocationResult result = chunk.reserve(memoryRequirements.size);
    if (result.found) {
        Block block = result.block;
        //mBuffersInfo[buffer] = {}; // For testing purpose
        mImagesInfo[image].block = block;
        mImagesInfo[image].chunkIndex = mChunksMap[memoryTypeIndex].size() - 1;
        mImagesInfo[image].memoryTypeIndex = memoryTypeIndex;

        vkBindImageMemory(mDevice, image, chunk.getMemory(), block.offset);
        return;
    }

    throw std::runtime_error("Unable to find enough memory");
}

void MemoryManager::freeBuffer(VkBuffer buffer) {
    BufferInfo bufferInfo = mBuffersInfo[buffer];

    if (!mChunksMap[bufferInfo.memoryTypeIndex][bufferInfo.chunkIndex].free(bufferInfo.block)) {
        throw std::runtime_error("Unable to find the block to free");
    }

    mBuffersInfo.erase(buffer);

    vkDestroyBuffer(mDevice, buffer, nullptr);
}

void MemoryManager::freeImage(VkImage image) {
    BufferInfo imageInfo = mImagesInfo[image];

    if (!mChunksMap[imageInfo.memoryTypeIndex][imageInfo.chunkIndex].free(imageInfo.block)) {
        throw std::runtime_error("Unable to find the block to free");
    }

    mImagesInfo.erase(image);
    
    vkDestroyImage(mDevice, image, nullptr);
}

void MemoryManager::mapMemory(VkBuffer buffer, VkDeviceSize size, void** data) {
    BufferInfo bufferInfo = mBuffersInfo[buffer];
    vkMapMemory(
        mDevice,
        mChunksMap[bufferInfo.memoryTypeIndex][bufferInfo.chunkIndex].getMemory(),
        bufferInfo.block.offset,
        size, 0, data);
}

void MemoryManager::unmapMemory(VkBuffer buffer) {
    BufferInfo bufferInfo = mBuffersInfo[buffer];
    vkUnmapMemory(mDevice, mChunksMap[bufferInfo.memoryTypeIndex][bufferInfo.chunkIndex].getMemory());
}

int32_t MemoryManager::findMemoryType(VkMemoryRequirements& memoryRequirements, VkMemoryPropertyFlags& properties) {
    int32_t memoryTypeIndex = -1;

    for (size_t i{0};i < mMemoryProperties.memoryTypeCount;++i) {
        if (memoryRequirements.memoryTypeBits & (1 << i)) {
            if ((properties & mMemoryProperties.memoryTypes[i].propertyFlags) == properties){
                memoryTypeIndex = i;
                break;
            }
        }
    }

    return memoryTypeIndex;
}