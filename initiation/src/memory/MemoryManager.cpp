#include <iostream>
#include <fstream>
#include <iomanip>

#include "memory/MemoryManager.hpp"

#include "PrintHelper.hpp"
#include "utils.hpp"

uint32_t MemoryManager::allocationSize = 64 * mega;
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

    initialAllocation();
}

void MemoryManager::printInfo() {
    for (size_t i{0};i < mMemoryTypeOccupations.size();++i) {
        uint32_t freeBlock{0}, occupiedBlock{0}, allocatedBlock{0};
        for (size_t j{0};j < mMemoryTypeOccupations[i].size();++j) {
            for (size_t blockIndex{0}; blockIndex < mMemoryTypeOccupations[i][j].blocks.size();++blockIndex) {
                allocatedBlock++;
                mMemoryTypeOccupations[i][j].blocks[blockIndex] ? occupiedBlock++ : freeBlock++;
            }
        }

        std::cout << "Memory Type #" << i << std::endl
            << "    " << allocatedBlock << " block(s) allocated" << std::endl
            << "    " << freeBlock << " free blocks" << std::endl
            << "    " << occupiedBlock << " occupied blocks" << std::endl;
    }
}

void MemoryManager::memoryCheckLog() {
    std::ofstream file;
    file.open("./memory.log", std::ios::out | std::ios::trunc);

    for (size_t i{0};i < mMemoryProperties.memoryHeapCount;++i) {
        file << "Memory Type #" << i << " : " << mDeviceMemoryAllocation[i].size() << " allocation(s)" << std::endl;
    }

    for (size_t i{0};i < mMemoryProperties.memoryHeapCount;++i) {
        for (size_t j{0};j < mMemoryTypeOccupations[i].size();++j) {
            for (size_t blockIndex{0};blockIndex < mMemoryTypeOccupations[i][j].blocks.size();++blockIndex) {
                if (mMemoryTypeOccupations[i][j].blocks[blockIndex]) {
                    file << "Memory Block #" << blockIndex << std::hex << " (0x"
                        << blockIndex * pageSize << " - 0x" << (blockIndex + 1) * pageSize
                        << ") not freed" << std::endl;
                }
            }
        }
    }
    file.close();
}

void MemoryManager::initialAllocation() {
    vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &mMemoryProperties);

    mDeviceMemoryAllocation.resize(mMemoryProperties.memoryTypeCount);
    mMemoryTypeOccupations.resize(mMemoryProperties.memoryTypeCount);

    for (size_t i{0};i < mMemoryProperties.memoryTypeCount;++i) {
        allocate(i);
    }
}

void MemoryManager::allocate(uint32_t memoryTypeIndex) {
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    allocInfo.allocationSize = allocationSize;

    VkDeviceMemory memoryAllocation;

    if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &memoryAllocation) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate memory");
    }
    mDeviceMemoryAllocation[memoryTypeIndex].push_back(memoryAllocation);
    mMemoryTypeOccupations[memoryTypeIndex].push_back(MemoryHeapOccupation(allocationSize / pageSize));

}

void MemoryManager::cleanup() {
    for (size_t i{0};i < mDeviceMemoryAllocation.size();i++) {
        for (size_t j{0};j < mDeviceMemoryAllocation[i].size();++j) {
            vkFreeMemory(mDevice, mDeviceMemoryAllocation[i][j], nullptr);
        }
    }
}

void MemoryManager::allocateForBuffer(VkBuffer buffer, VkMemoryRequirements& memoryRequirements, VkMemoryPropertyFlags properties) {
    int32_t memoryTypeIndex = findMemoryType(memoryRequirements, properties);

    if (memoryTypeIndex == -1) {
        throw std::runtime_error("Unable to find a suitable memory type");
    }

    uint32_t blockCount = getBlockCount(memoryRequirements);

    if (memoryRequirements.alignment <= pageSize) {
        int32_t memoryTypeOffset, pageOffset;
        std::tie<int32_t, int32_t>(memoryTypeOffset, pageOffset) = findSuitableMemoryBlock(blockCount, memoryTypeIndex);

        if (memoryTypeOffset == -1) {
            allocate(memoryTypeIndex);
            std::tie<int32_t, int32_t>(memoryTypeOffset, pageOffset) = findSuitableMemoryBlock(blockCount, memoryTypeIndex);
        }

        for (uint32_t i{static_cast<uint32_t>(pageOffset)};i < pageOffset + blockCount;++i) {
            mMemoryTypeOccupations[memoryTypeIndex][memoryTypeOffset].blocks[i] = true;
            mMemoryTypeOccupations[memoryTypeIndex][memoryTypeOffset].blocks[i].buffer = buffer;
        }

        mBuffersInfo[buffer] = {
            static_cast<uint32_t>(memoryTypeIndex),
            static_cast<uint32_t>(memoryTypeOffset),
            static_cast<uint32_t>(pageOffset),
            0,
            blockCount};

        vkBindBufferMemory(mDevice, buffer, mDeviceMemoryAllocation[memoryTypeIndex][memoryTypeOffset], pageOffset * pageSize);
    } else {
        throw std::runtime_error("Not implemented");
    }
}

void MemoryManager::allocateForImage(VkImage image, VkMemoryRequirements& memoryRequirements, VkMemoryPropertyFlags properties) {
    int32_t memoryTypeIndex = findMemoryType(memoryRequirements, properties);

    if (memoryTypeIndex == -1) {
        throw std::runtime_error("Unable to find a suitable memory type");
    }

    uint32_t blockCount = getBlockCount(memoryRequirements);

    if (memoryRequirements.alignment <= pageSize) {
        int32_t memoryTypeOffset, pageOffset;
        std::tie<int32_t, int32_t>(memoryTypeOffset, pageOffset) = findSuitableMemoryBlock(blockCount, memoryTypeIndex);

        if (memoryTypeOffset == -1) {
            allocate(memoryTypeIndex);
            std::tie<int32_t, int32_t>(memoryTypeOffset, pageOffset) = findSuitableMemoryBlock(blockCount, memoryTypeIndex);
        }

        for (uint32_t i{static_cast<uint32_t>(pageOffset)};i < pageOffset + blockCount;++i) {
            mMemoryTypeOccupations[memoryTypeIndex][memoryTypeOffset].blocks[i] = true;
            mMemoryTypeOccupations[memoryTypeIndex][memoryTypeOffset].blocks[i].image = image;
        }

        mImagesInfo[image] = {
            static_cast<uint32_t>(memoryTypeIndex),
            static_cast<uint32_t>(memoryTypeOffset),
            static_cast<uint32_t>(pageOffset),
            0,
            blockCount
        };
        vkBindImageMemory(mDevice, image, mDeviceMemoryAllocation[memoryTypeIndex][memoryTypeOffset], pageOffset * pageSize);
    } else {
        throw std::runtime_error("Not implemented");
    }
}

void MemoryManager::freeBuffer(VkBuffer buffer) {
    BufferInfo bufferInfo = mBuffersInfo[buffer];

    for (uint32_t i{0};i < bufferInfo.blockCount;i++) {
        mMemoryTypeOccupations[bufferInfo.memoryTypeIndex][bufferInfo.memoryTypeOffset].blocks[bufferInfo.pageOffset + i].buffer = VK_NULL_HANDLE;
        mMemoryTypeOccupations[bufferInfo.memoryTypeIndex][bufferInfo.memoryTypeOffset].blocks[bufferInfo.pageOffset + i].occupied = false;
    }

    vkDestroyBuffer(mDevice, buffer, nullptr);
}

void MemoryManager::freeImage(VkImage image) {
    BufferInfo imageInfo = mImagesInfo[image];

    for (uint32_t i{0};i < imageInfo.blockCount;i++) {
        mMemoryTypeOccupations[imageInfo.memoryTypeIndex][imageInfo.memoryTypeOffset].blocks[imageInfo.pageOffset + i].image = VK_NULL_HANDLE;
        mMemoryTypeOccupations[imageInfo.memoryTypeIndex][imageInfo.memoryTypeOffset].blocks[imageInfo.pageOffset + i].occupied = false;
    }
    
    vkDestroyImage(mDevice, image, nullptr);
}

void MemoryManager::mapMemory(VkBuffer buffer, VkDeviceSize size, void** data) {
    BufferInfo bufferInfo = mBuffersInfo[buffer];
    vkMapMemory(
        mDevice,
        mDeviceMemoryAllocation[bufferInfo.memoryTypeIndex][bufferInfo.memoryTypeOffset],
        bufferInfo.pageOffset * pageSize + bufferInfo.alignment,
        size, 0, data);
}

void MemoryManager::unmapMemory(VkBuffer buffer) {
    BufferInfo bufferInfo = mBuffersInfo[buffer];
    vkUnmapMemory(mDevice, mDeviceMemoryAllocation[bufferInfo.memoryTypeIndex][bufferInfo.memoryTypeOffset]);
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

std::tuple<int32_t, int32_t> MemoryManager::findSuitableMemoryBlock(uint32_t blockCount, uint32_t memoryTypeIndex) {
    int32_t memoryTypeOffset = -1;
    int32_t pageOffset = -1;

    for (size_t i{0};i < mMemoryTypeOccupations[memoryTypeIndex].size();++i) {
        pageOffset = findSuitableMemoryBlock(blockCount, memoryTypeIndex, i);

        if (pageOffset != -1) {
            memoryTypeOffset = i;
            break;
        }
    }

    return std::make_tuple(memoryTypeOffset, pageOffset);
}

int32_t MemoryManager::findSuitableMemoryBlock(uint32_t blockCount, uint32_t memoryTypeIndex, uint32_t memoryTypeOffset) {
    size_t i = 0;

    while(i < mMemoryTypeOccupations[memoryTypeIndex][memoryTypeOffset].blocks.size()) {
        uint32_t start = i;
        uint32_t end = i + blockCount;

        bool enough = true;
        for (uint32_t j{start};j < end;++j) {
            if (mMemoryTypeOccupations[memoryTypeIndex][memoryTypeOffset].blocks[j]) {
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

uint32_t MemoryManager::getBlockCount(VkMemoryRequirements& memoryRequirements) {
    uint32_t blockCount = memoryRequirements.size / pageSize;
    if (memoryRequirements.size & (pageSize - 1)) {
        return blockCount + 1;
    } else {
        return blockCount;
    }
}

uint32_t MemoryManager::getAlignment(VkMemoryRequirements& memoryRequirements) {
    uint32_t requiredAlignment = memoryRequirements.alignment;
    uint32_t multiple = requiredAlignment / pageSize;
}