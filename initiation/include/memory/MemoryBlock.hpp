#ifndef MEMORYBLOCK
#define MEMORYBLOCK

#include <vulkan/vulkan.h>

struct MemoryBlock {
    VkBuffer buffer = VK_NULL_HANDLE;
    bool occupied = false;

    operator bool() {
        return occupied;
    }

    MemoryBlock& operator=(bool o) {
        occupied = o;
        return *this;
    }
};

#endif