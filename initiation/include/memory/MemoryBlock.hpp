#ifndef MEMORYBLOCK
#define MEMORYBLOCK

#include <string>

#include <vulkan/vulkan.h>

struct MemoryBlock {
    VkBuffer buffer = VK_NULL_HANDLE;
    VkImage image = VK_NULL_HANDLE;
    bool occupied = false;
    std::string name;

    operator bool() {
        return occupied;
    }

    MemoryBlock& operator=(bool o) {
        occupied = o;
        return *this;
    }
};

#endif