#ifndef BUFFERINFO
#define BUFFERINFO

#include <cstdint>

struct BufferInfo {
    uint32_t memoryHeapIndex;
    uint32_t memoryHeapOffset;
    uint32_t offset;
    uint32_t blockCount;
};

#endif