#include <iostream>
#include <fstream>

#include "memory/MemoryManager.hpp"

#include "PrintHelper.hpp"
#include "utils.hpp"

uint32_t MemoryManager::initialAllocationSize = 256 * mega;
uint32_t MemoryManager::pageSize = 4 * kilo;

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

    std::ofstream file;
    file.open("./memory.log", std::ios::out | std::ios::trunc);
    for (size_t i{0};i < mMemoryProperties.memoryHeapCount;++i) {
        file << "Memory Heap #" << i << std::endl;
        for (size_t blockIndex{0};blockIndex < mMemoryOccupations[i].size();++blockIndex) {
            file << mMemoryOccupations[i][blockIndex] ? "1" : "0";
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
    int32_t memoryTypeIndex = -1;
    for (size_t i{0};i < mMemoryProperties.memoryTypeCount;++i) {
        if (memoryRequirements.memoryTypeBits & (1 << i)) {
            memoryTypeIndex = i;
        }
    }
    if (memoryTypeIndex == -1) {
        throw std::runtime_error("Unable to find a suitable memory type");
    }

    std::cout   << "Required size: " << memoryRequirements.size << std::endl
                << "Block count: " << getBlockCount(memoryRequirements.size) << std::endl;

    uint32_t blockCount = getBlockCount(memoryRequirements.size);
    uint32_t memoryHeapIndex = mMemoryProperties.memoryTypes[memoryTypeIndex].heapIndex;
    
    size_t i = 0;
    size_t offset;
    bool found = false;
    while(i < mMemoryOccupations[memoryHeapIndex].size()) {
        offset = i;
        size_t count = 0;
        while (count < blockCount && !mMemoryOccupations[memoryHeapIndex][i]) {
            count++;
            i++;
        }

        if (count == blockCount) {
            found = true;
            break;
        }
        i++;
    }

    if (!found) {
        throw std::runtime_error("Failed to find free blocks");
    }

    for (size_t i{offset};i < offset + blockCount;++i) {
        mMemoryOccupations[memoryHeapIndex][i] = true;
    }
    printInfo();
}

uint32_t MemoryManager::getBlockCount(uint32_t requiredSize) {
    uint32_t blockCount = requiredSize / pageSize;
    if (requiredSize & (pageSize - 1)) {
        return blockCount + 1;
    } else {
        return blockCount;
    }
}