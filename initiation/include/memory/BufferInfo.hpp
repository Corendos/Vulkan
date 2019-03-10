#ifndef BUFFERINFO
#define BUFFERINFO

#include <cstdint>

#include "memory/Block.hpp"

struct BufferInfo {
    Block block;
    uint32_t chunkIndex;
    uint32_t memoryTypeIndex;
};

#endif