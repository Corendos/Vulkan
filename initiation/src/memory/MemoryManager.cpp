#include <iostream>

#include "memory/MemoryManager.hpp"

#include "PrintHelper.hpp"
#include "utils.hpp"

MemoryManager::MemoryManager(VkPhysicalDevice& physicalDevice, VkDevice& device) :
    mDevice(device), mPhysicalDevice(physicalDevice) {}

void MemoryManager::init() {
    findSuitableMemoryHeap();
    initialAllocation();
}

void MemoryManager::printInfo() {
    std::cout << "mMemoryHeapIndex: " << mMemoryHeapIndex << std::endl;
    std::cout << "mMemoryTypeIndex: " << mMemoryTypeIndex << std::endl;
}

void MemoryManager::findSuitableMemoryHeap() {
    VkPhysicalDeviceMemoryProperties properties;
    vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &properties);

    for (uint32_t i{0}; i < properties.memoryTypeCount;++i) {
        if (properties.memoryTypes[i].propertyFlags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
            mMemoryHeapIndex = properties.memoryTypes[i].heapIndex;
            mMemoryTypeIndex = i;
            break;
        }
    }
}

void MemoryManager::initialAllocation() {
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.memoryTypeIndex = mMemoryTypeIndex;
    allocInfo.allocationSize = 1 * giga;

    if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &mMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate memory");
    }
}

void MemoryManager::cleanup() {
    vkFreeMemory(mDevice, mMemory, nullptr);
}