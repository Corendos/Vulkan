#include <iostream>
#include <fstream>
#include <iomanip>

#include "memory/MemoryManager.hpp"

#include "PrintHelper.hpp"
#include "utils.hpp"

uint32_t MemoryManager::allocationSize = 256;
uint32_t MemoryManager::pageSize = 128;

MemoryManager::MemoryManager(VkPhysicalDevice& physicalDevice, VkDevice& device) :
    mDevice(device), mPhysicalDevice(physicalDevice) {
}

void MemoryManager::init() {
    initialAllocation();
}

void MemoryManager::printInfo() {
    for (size_t i{0};i < mMemoryHeapOccupations.size();++i) {
        uint32_t freeBlock{0}, occupiedBlock{0}, allocatedBlock{0};
        for (size_t j{0};j < mMemoryHeapOccupations[i].size();++j) {
            for (size_t blockIndex{0}; blockIndex < mMemoryHeapOccupations[i][j].blocks.size();++blockIndex) {
                allocatedBlock++;
                mMemoryHeapOccupations[i][j].blocks[blockIndex] ? occupiedBlock++ : freeBlock++;
            }
        }

        std::cout << "Memory Heap #" << i << std::endl
            << "    " << allocatedBlock << " block(s) allocated" << std::endl
            << "    " << freeBlock << " free blocks" << std::endl
            << "    " << occupiedBlock << " occupied blocks" << std::endl;
    }
}

void MemoryManager::memoryCheckLog() {
    std::ofstream file;
    file.open("./memory.log", std::ios::out | std::ios::trunc);

    for (size_t i{0};i < mMemoryProperties.memoryHeapCount;++i) {
        for (size_t j{0};j < mMemoryHeapOccupations[i].size();++j) {
            for (size_t blockIndex{0};blockIndex < mMemoryHeapOccupations[i][j].blocks.size();++blockIndex) {
                if (mMemoryHeapOccupations[i][j].blocks[blockIndex]) {
                    file << "Memory Block #" << blockIndex << std::hex << " (0x"
                        << blockIndex * pageSize << " - 0x" << (blockIndex + 1) * pageSize
                        << ") not freed" << std::endl;
                }
            }
            file << std::endl;
        }
    }
    file.close();
}

void MemoryManager::initialAllocation() {
    vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &mMemoryProperties);

    mDeviceMemoryAllocation.resize(mMemoryProperties.memoryHeapCount);
    mMemoryHeapOccupations.resize(mMemoryProperties.memoryHeapCount);

    for (size_t i{0};i < mMemoryProperties.memoryTypeCount;++i) {
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.memoryTypeIndex = i;
        allocInfo.allocationSize = allocationSize;

        uint32_t memoryHeapIndex = mMemoryProperties.memoryTypes[i].heapIndex;
        VkDeviceMemory memoryAllocation;

        if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &memoryAllocation) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate memory");
        }
        mDeviceMemoryAllocation[memoryHeapIndex].push_back(memoryAllocation);
        mMemoryHeapOccupations[memoryHeapIndex].push_back(MemoryHeapOccupation(allocationSize / pageSize));
    }
}

void MemoryManager::allocate(uint32_t memoryTypeIndex) {
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    allocInfo.allocationSize = allocationSize;

    uint32_t memoryHeapIndex = mMemoryProperties.memoryTypes[memoryTypeIndex].heapIndex;
    VkDeviceMemory memoryAllocation;

    if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &memoryAllocation) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate memory");
    }
    mDeviceMemoryAllocation[memoryHeapIndex].push_back(memoryAllocation);
    mMemoryHeapOccupations[memoryHeapIndex].push_back(MemoryHeapOccupation(allocationSize / pageSize));

}

void MemoryManager::cleanup() {
    for (size_t i{0};i < mDeviceMemoryAllocation.size();i++) {
        for (size_t j{0};j < mDeviceMemoryAllocation[i].size();++j) {
            vkFreeMemory(mDevice, mDeviceMemoryAllocation[i][j], nullptr);
        }
    }
}

void MemoryManager::allocateForBuffer(VkBuffer& buffer, VkMemoryRequirements& memoryRequirements) {
    int32_t memoryTypeIndex = findMemoryType(memoryRequirements);

    if (memoryTypeIndex == -1) {
        throw std::runtime_error("Unable to find a suitable memory type");
    }

    uint32_t blockCount = getBlockCount(memoryRequirements.size);
    uint32_t memoryHeapIndex = mMemoryProperties.memoryTypes[memoryTypeIndex].heapIndex;

    int32_t memoryHeapOffset, offset;
    std::tie<int32_t, int32_t>(memoryHeapOffset, offset) = findSuitableMemoryBlock(blockCount, memoryHeapIndex);

    if (memoryHeapOffset == -1) {
        // TODO FIXME: Do another allocation
        allocate(memoryTypeIndex);
        std::tie<int32_t, int32_t>(memoryHeapOffset, offset) = findSuitableMemoryBlock(blockCount, memoryHeapIndex);
    }

    for (uint32_t i{static_cast<uint32_t>(offset)};i < offset + blockCount;++i) {
        mMemoryHeapOccupations[memoryHeapIndex][memoryHeapOffset].blocks[offset] = true;
        mMemoryHeapOccupations[memoryHeapIndex][memoryHeapOffset].blocks[offset].buffer = buffer;
    }

    mBuffersInfo[buffer] = {memoryHeapIndex, memoryHeapOffset, offset, blockCount};

    vkBindBufferMemory(mDevice, buffer, mDeviceMemoryAllocation[memoryHeapIndex][memoryHeapOffset], offset * pageSize);
}

void MemoryManager::freeBuffer(VkBuffer& buffer) {
    BufferInfo bufferInfo = mBuffersInfo[buffer];

    for (uint32_t i{0};i < bufferInfo.blockCount;i++) {
        mMemoryHeapOccupations[bufferInfo.memoryHeapIndex][bufferInfo.memoryHeapOffset].blocks[bufferInfo.offset + i].buffer = VK_NULL_HANDLE;
        mMemoryHeapOccupations[bufferInfo.memoryHeapIndex][bufferInfo.memoryHeapOffset].blocks[bufferInfo.offset + i].occupied = false;
    }

    vkDestroyBuffer(mDevice, buffer, nullptr);
}

void MemoryManager::mapMemory(VkBuffer& buffer, VkDeviceSize size, void** data) {
    BufferInfo bufferInfo = mBuffersInfo[buffer];
    vkMapMemory(
        mDevice,
        mDeviceMemoryAllocation[bufferInfo.memoryHeapIndex][bufferInfo.memoryHeapOffset],
        bufferInfo.offset * pageSize,
        size, 0, data);
}

void MemoryManager::unmapMemory(VkBuffer& buffer) {
    BufferInfo bufferInfo = mBuffersInfo[buffer];
    vkUnmapMemory(mDevice, mDeviceMemoryAllocation[bufferInfo.memoryHeapIndex][bufferInfo.memoryHeapOffset]);
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

std::tuple<int32_t, int32_t> MemoryManager::findSuitableMemoryBlock(uint32_t blockCount, uint32_t memoryHeapIndex) {
    int32_t memoryHeapOffset = -1;
    int32_t offset = -1;

    for (size_t i{0};i < mMemoryHeapOccupations[memoryHeapIndex].size();++i) {
        offset = findSuitableMemoryBlock(blockCount, memoryHeapIndex, i);

        if (offset != -1) {
            memoryHeapOffset = i;
            break;
        }
    }

    return std::make_tuple(memoryHeapOffset, offset);
}

int32_t MemoryManager::findSuitableMemoryBlock(uint32_t blockCount, uint32_t memoryHeapIndex, uint32_t memoryHeapOffset) {
    size_t i = 0;

    while(i < mMemoryHeapOccupations[memoryHeapIndex][memoryHeapOffset].blocks.size()) {
        uint32_t start = i;
        uint32_t end = i + blockCount;

        bool enough = true;
        for (uint32_t j{start};j < end;++j) {
            if (mMemoryHeapOccupations[memoryHeapIndex][memoryHeapOffset].blocks[j]) {
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