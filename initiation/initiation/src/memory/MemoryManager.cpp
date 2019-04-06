#include <iostream>
#include <fstream>
#include <iomanip>

#include "memory/MemoryManager.hpp"

#include "PrintHelper.hpp"
#include "utils.hpp"

uint32_t MemoryManager::allocationSize = 32 * mega;
uint32_t MemoryManager::pageSize = 4 * kilo;

MemoryManager::MemoryManager(vk::PhysicalDevice& physicalDevice, vk::Device& device) :
    mDevice(device), mPhysicalDevice(physicalDevice) {
}

void MemoryManager::init() {
    vk::PhysicalDeviceProperties properties = mPhysicalDevice.getProperties();

    if (pageSize < properties.limits.bufferImageGranularity) {
        pageSize = properties.limits.bufferImageGranularity;
    }

    mMemoryProperties = mPhysicalDevice.getMemoryProperties();
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
    vk::MemoryAllocateInfo allocateInfo;
    allocateInfo.setMemoryTypeIndex(memoryTypeIndex);
    allocateInfo.setAllocationSize(allocationSize);

    vk::DeviceMemory memoryAllocation = mDevice.allocateMemory(allocateInfo);

    if (mChunksMap.find(memoryTypeIndex) == mChunksMap.end()) {
        mChunksMap[memoryTypeIndex] = std::vector<Chunk>();
    }

    mChunksMap[memoryTypeIndex].push_back(Chunk(memoryAllocation, allocationSize, pageSize));
}

void MemoryManager::cleanup() {
    for (auto& pair : mChunksMap) {
        for (auto& chunk : pair.second) {
            mDevice.freeMemory(chunk.getMemory());
        }
    }
}

void MemoryManager::allocateForBuffer(vk::Buffer buffer,
                                      vk::MemoryRequirements& memoryRequirements,
                                      vk::MemoryPropertyFlags properties,
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
            mBuffersInfo[buffer].block = block;
            mBuffersInfo[buffer].chunkIndex = i;
            mBuffersInfo[buffer].memoryTypeIndex = memoryTypeIndex;

            mDevice.bindBufferMemory(buffer, chunk.getMemory(), block.offset);
            return;
        }
    }

    // If we come to this point, this means we need to allocate a new Chunk of memory because the previous one are full
    allocate(memoryTypeIndex);
    Chunk& chunk = mChunksMap[memoryTypeIndex].back();
    AllocationResult result = chunk.reserve(memoryRequirements.size);
    if (result.found) {
        Block block = result.block;
        mBuffersInfo[buffer].block = block;
        mBuffersInfo[buffer].chunkIndex = mChunksMap[memoryTypeIndex].size() - 1;
        mBuffersInfo[buffer].memoryTypeIndex = memoryTypeIndex;

        mDevice.bindBufferMemory(buffer, chunk.getMemory(), block.offset);
        return;
    }

    throw std::runtime_error("Unable to find enough memory");
}

void MemoryManager::allocateForImage(vk::Image image,
                                     vk::MemoryRequirements& memoryRequirements,
                                     vk::MemoryPropertyFlags properties,
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
            mImagesInfo[image].block = block;
            mImagesInfo[image].chunkIndex = i;
            mImagesInfo[image].memoryTypeIndex = memoryTypeIndex;

            mDevice.bindImageMemory(image, chunk.getMemory(), block.offset);
            return;
        }
    }

    // If we come to this point, this means we need to allocate a new Chunk of memory because the previous one are full
    allocate(memoryTypeIndex);
    Chunk& chunk = mChunksMap[memoryTypeIndex].back();
    AllocationResult result = chunk.reserve(memoryRequirements.size);
    if (result.found) {
        Block block = result.block;
        mImagesInfo[image].block = block;
        mImagesInfo[image].chunkIndex = mChunksMap[memoryTypeIndex].size() - 1;
        mImagesInfo[image].memoryTypeIndex = memoryTypeIndex;

        mDevice.bindImageMemory(image, chunk.getMemory(), block.offset);
        return;
    }

    throw std::runtime_error("Unable to find enough memory");
}

void MemoryManager::freeBuffer(vk::Buffer buffer) {
    BufferInfo bufferInfo = mBuffersInfo[buffer];

    if (!mChunksMap[bufferInfo.memoryTypeIndex][bufferInfo.chunkIndex].free(bufferInfo.block)) {
        throw std::runtime_error("Unable to find the block to free");
    }

    mBuffersInfo.erase(buffer);

    mDevice.destroyBuffer(buffer);
}

void MemoryManager::freeImage(vk::Image image) {
    BufferInfo imageInfo = mImagesInfo[image];

    if (!mChunksMap[imageInfo.memoryTypeIndex][imageInfo.chunkIndex].free(imageInfo.block)) {
        throw std::runtime_error("Unable to find the block to free");
    }

    mImagesInfo.erase(image);
    
    mDevice.destroyImage(image);
}

void MemoryManager::mapMemory(vk::Buffer buffer, vk::DeviceSize size, void** data) {
    BufferInfo bufferInfo = mBuffersInfo[buffer];
    *data = mDevice.mapMemory(
        mChunksMap[bufferInfo.memoryTypeIndex][bufferInfo.chunkIndex].getMemory(),
        bufferInfo.block.offset,
        size);
}

void MemoryManager::unmapMemory(vk::Buffer buffer) {
    BufferInfo bufferInfo = mBuffersInfo[buffer];
    mDevice.unmapMemory(mChunksMap[bufferInfo.memoryTypeIndex][bufferInfo.chunkIndex].getMemory());
}

int32_t MemoryManager::findMemoryType(vk::MemoryRequirements& memoryRequirements, vk::MemoryPropertyFlags& properties) {
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