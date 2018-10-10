#ifndef PRINTHELPER
#define PRINTHELPER

#include <string>
#include <sstream>

#include <vulkan/vulkan.h>

typedef enum {
    VK_MEMORY_HEAP_FLAGS,
    VK_MEMORY_PROPERTY_FLAGS,
} FlagsType;

class PrintHelper {
    public:
        static char fill;
        static std::string toString(VkPhysicalDeviceMemoryProperties& properties, size_t tabCount = 0);

        static std::string toString(VkMemoryHeap memoryHeap, size_t tabCount = 0);
        static std::string toString(VkMemoryHeap* memoryHeap, size_t count, size_t tabCount = 0);

        static std::string toString(VkMemoryType memoryType, size_t tabCount = 0);
        static std::string toString(VkMemoryType* memoryType, size_t count, size_t tabCount = 0);

        static std::string toString(VkFlags& flags, FlagsType type, size_t tabCount = 0);

        static std::string toStringVkMemoryHeapFlags(VkMemoryHeapFlags memoryHeapFlags, size_t tabCount = 0);
        static std::string toStringVkMemoryPropertyFlags(VkMemoryPropertyFlags memoryPropertyFlags, size_t tabCount = 0);
};

#endif