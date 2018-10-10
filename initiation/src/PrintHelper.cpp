#include "PrintHelper.hpp"

char PrintHelper::fill = '.';

std::string PrintHelper::toString(VkPhysicalDeviceMemoryProperties& properties, size_t tabCount) {
    std::ostringstream stream;
    stream
        << "PhysicalDeviceMemoryProperties" << std::endl
        << std::string(tabCount, PrintHelper::fill) << "Memory Heap Count: " << properties.memoryHeapCount << std::endl
        << std::string(tabCount, PrintHelper::fill) << "Memory Heap:" << std::endl
        << PrintHelper::toString(properties.memoryHeaps, properties.memoryHeapCount, tabCount + 4)
        << std::string(tabCount, PrintHelper::fill) << "Memory Type Count: " << properties.memoryTypeCount << std::endl
        << std::string(tabCount, PrintHelper::fill) << "Memory Type:" << std::endl
        << PrintHelper::toString(properties.memoryTypes, properties.memoryTypeCount, tabCount + 4);
    return stream.str();
}

std::string PrintHelper::toString(VkMemoryHeap memoryHeap, size_t tabCount) {
    std::ostringstream stream;
    stream
        << std::string(tabCount, PrintHelper::fill) << "Size: " << memoryHeap.size << std::endl
        << std::string(tabCount, PrintHelper::fill) << "Flags: " << std::endl
        << PrintHelper::toStringVkMemoryHeapFlags(memoryHeap.flags, tabCount + 4);
    return stream.str();
}

std::string PrintHelper::toString(VkMemoryHeap* memoryHeap, size_t count, size_t tabCount) {
    std::ostringstream stream;
    for (size_t i{0};i < count;++i) {
        stream << std::string(tabCount, PrintHelper::fill) << "[" << i << "]" << std::endl
            << PrintHelper::toString(memoryHeap[i], tabCount);
    }
    return stream.str();
}

std::string PrintHelper::toString(VkMemoryType memoryType, size_t tabCount) {
    std::ostringstream stream;
    stream
        << std::string(tabCount, PrintHelper::fill) << "Heap Index: " << memoryType.heapIndex << std::endl
        << std::string(tabCount, PrintHelper::fill) << "Property Flags:" << std::endl
        << PrintHelper::toStringVkMemoryPropertyFlags(memoryType.propertyFlags, tabCount + 4);
    return stream.str();
}

std::string PrintHelper::toString(VkMemoryType* memoryType, size_t count, size_t tabCount) {
    std::ostringstream stream;
    for (size_t i{0};i < count;++i) {
        stream << PrintHelper::toString(memoryType[i], tabCount + 4);
    }
    return stream.str();
}

std::string PrintHelper::toStringVkMemoryHeapFlags(VkMemoryHeapFlags memoryHeapFlags, size_t tabCount) {
    std::ostringstream stream;
    if (memoryHeapFlags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
        stream << std::string(tabCount, PrintHelper::fill) << "VK_MEMORY_HEAP_DEVICE_LOCAL_BIT" << std::endl;
    }

    if (memoryHeapFlags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT) {
        stream << std::string(tabCount, PrintHelper::fill) << "VK_MEMORY_HEAP_MULTI_INSTANCE_BIT" << std::endl;
    }
    return stream.str();
}

std::string PrintHelper::toStringVkMemoryPropertyFlags(VkMemoryPropertyFlags memoryPropertyFlags, size_t tabCount) {
    std::ostringstream stream;
    if (memoryPropertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        stream << std::string(tabCount, PrintHelper::fill) << "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT" << std::endl;
    if (memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        stream << std::string(tabCount, PrintHelper::fill) << "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT" << std::endl;
    if (memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
        stream << std::string(tabCount, PrintHelper::fill) << "VK_MEMORY_PROPERTY_HOST_COHERENT_BIT" << std::endl;
    if (memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
        stream << std::string(tabCount, PrintHelper::fill) << "VK_MEMORY_PROPERTY_HOST_CACHED_BIT" << std::endl;
    if (memoryPropertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
        stream << std::string(tabCount, PrintHelper::fill) << "VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT" << std::endl;
    if (memoryPropertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT)
        stream << std::string(tabCount, PrintHelper::fill) << "VK_MEMORY_PROPERTY_PROTECTED_BIT" << std::endl;
    return stream.str();
}

std::string PrintHelper::toString(VkFlags& flags, FlagsType type, size_t tabCount) {
    switch(type) {
        case VK_MEMORY_HEAP_FLAGS:
            return toStringVkMemoryHeapFlags(flags, tabCount);
        break;

        case VK_MEMORY_PROPERTY_FLAGS:
            return toStringVkMemoryPropertyFlags(flags, tabCount);
        break;

        default:
            std::ostringstream stream;
            stream << std::endl;
            return stream.str();
        break;
    }
}