#include <iostream>
#include <fstream>
#include <iomanip>

#include "memory/MemoryManager.hpp"

#include "PrintHelper.hpp"
#include "utils.hpp"

uint32_t MemoryManager::initialAllocationSize = 256;
uint32_t MemoryManager::pageSize = 128;

MemoryManager::MemoryManager(VkPhysicalDevice& physicalDevice, VkDevice& device) :
    mDevice(device), mPhysicalDevice(physicalDevice) {
}

void MemoryManager::init() {
    initialAllocation();
}

void MemoryManager::printInfo() {
    for (size_t i{0};i < mMemoryOccupations.size();++i) {
        uint32_t freeBlock{0}, occupiedBlock{0};
        for (size_t blockIndex{0}; blockIndex < mMemoryOccupations[i].size();++blockIndex) {
            mMemoryOccupations[i][blockIndex] ? occupiedBlock++ : freeBlock++;
        }
        std::cout << "Memory Heap #" << i << std::endl
            << "    " << mMemoryOccupations[i].size() << " block(s) allocated" << std::endl
            << "    " << freeBlock << " free blocks" << std::endl
            << "    " << occupiedBlock << " occupied blocks" << std::endl;
    }
}

void MemoryManager::memoryCheckLog() {
    std::ofstream file;
    file.open("./memory.log", std::ios::out | std::ios::trunc);

    for (size_t i{0};i < mMemoryProperties.memoryHeapCount;++i) {
        for (size_t blockIndex{0};blockIndex < mMemoryOccupations[i].size();++blockIndex) {
            if (mMemoryOccupations[i][blockIndex]) {
                file << "Memory Block #" << blockIndex << std::hex << " (0x"
                    << blockIndex * pageSize << " - 0x" << (blockIndex + 1) * pageSize
                    << ") not freed" << std::endl;
            }
        }
        file << std::endl;
    }
    file.close();
}

void MemoryManager::initialAllocation() {
    vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &mMemoryProperties);

    mMemoryHeaps.resize(mMemoryProperties.memoryHeapCount);
    mMemoryOccupations.resize(mMemoryProperties.memoryHeapCount);

    for (size_t i{0};i < mMemoryProperties.memoryTypeCount;++i) {
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.memoryTypeIndex = i;
        allocInfo.allocationSize = initialAllocationSize;

        uint32_t memoryHeapIndex = mMemoryProperties.memoryTypes[i].heapIndex;

        if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &mMemoryHeaps[memoryHeapIndex]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate memory");
        }
        size_t currentSize = mMemoryOccupations[memoryHeapIndex].size();
        mMemoryOccupations[memoryHeapIndex].resize(currentSize + initialAllocationSize / (4 * kilo));
    }
}

void MemoryManager::cleanup() {
    for (size_t i{0};i < mMemoryHeaps.size();i++) {
        vkFreeMemory(mDevice, mMemoryHeaps[i], nullptr);
    }
}

void MemoryManager::allocateForBuffer(VkBuffer& buffer, VkMemoryRequirements& memoryRequirements) {
    int32_t memoryTypeIndex = findMemoryType(memoryRequirements);

    if (memoryTypeIndex == -1) {
        throw std::runtime_error("Unable to find a suitable memory type");
    }

    uint32_t blockCount = getBlockCount(memoryRequirements.size);
    uint32_t memoryHeapIndex = mMemoryProperties.memoryTypes[memoryTypeIndex].heapIndex;

    int32_t offset = findSuitableMemoryBlock(blockCount, memoryHeapIndex);

    if (offset == -1) {
        throw std::runtime_error("Unable to find enough space");
    }

    for (uint32_t i{static_cast<uint32_t>(offset)};i < offset + blockCount;++i) {
        mMemoryOccupations[memoryHeapIndex][i] = true;
        mMemoryOccupations[memoryHeapIndex][i].buffer = buffer;
    }

    mBuffersInfo[buffer] = {memoryHeapIndex, offset, blockCount};

    vkBindBufferMemory(mDevice, buffer, mMemoryHeaps[memoryHeapIndex], offset * pageSize);
}

void MemoryManager::freeBuffer(VkBuffer& buffer) {
    BufferInfo bufferInfo = mBuffersInfo[buffer];

    for (uint32_t i{0};i < bufferInfo.blockCount;i++) {
        mMemoryOccupations[bufferInfo.memoryHeapIndex][bufferInfo.offset + i].buffer = VK_NULL_HANDLE;
        mMemoryOccupations[bufferInfo.memoryHeapIndex][bufferInfo.offset + i].occupied = false;
    }

    vkDestroyBuffer(mDevice, buffer, nullptr);
}

void MemoryManager::mapMemory(VkBuffer& buffer, VkDeviceSize size, void** data) {
    BufferInfo bufferInfo = mBuffersInfo[buffer];
    vkMapMemory(
        mDevice,
        mMemoryHeaps[bufferInfo.memoryHeapIndex],
        bufferInfo.offset * pageSize,
        size, 0, data);
}

void MemoryManager::unmapMemory(VkBuffer& buffer) {
    BufferInfo bufferInfo = mBuffersInfo[buffer];
    vkUnmapMemory(mDevice, mMemoryHeaps[bufferInfo.memoryHeapIndex]);
}

int32_t MemoryManager::findMemoryType(VkMemoryRequirements& memoryRequirements) {
    int32_t memoryTypeIndex = -1;

    for (size_t i{0};i < mMemoryProperties.memoryTypeCount;++i) {
        if (memoryRequirements.memoryTypeBits & (1 << i)) {
            memoryTypeIndex = i;
        }
    }

    return memoryTypeIndex;
}

int32_t MemoryManager::findSuitableMemoryBlock(uint32_t blockCount, uint32_t memoryHeapIndex) {
    size_t i = 0;

    while(i < mMemoryOccupations[memoryHeapIndex].size()) {
        uint32_t start = i;
        uint32_t end = i + blockCount;

        bool enough = true;
        for (uint32_t j{start};j < end;++j) {
            if (mMemoryOccupations[memoryHeapIndex][j]) {
                i = j + 1;
                enough = false;
                break;
            }
        }

        if (enough) {
            return static_cast<int32_t>(start);
        }
    }

    return -1;
}

uint32_t MemoryManager::getBlockCount(uint32_t requiredSize) {
    uint32_t blockCount = requiredSize / pageSize;
    if (requiredSize & (pageSize - 1)) {
        return blockCount + 1;
    } else {
        return blockCount;
    }
}