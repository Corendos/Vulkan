#ifndef BUFFERINFO
#define BUFFERINFO

#include <cstdint>

struct BufferInfo {
    uint32_t memoryTypeIndex;
    uint32_t memoryTypeOffset;
    uint32_t offset;
    uint32_t blockCount;
};

#endif