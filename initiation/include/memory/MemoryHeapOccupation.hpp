#ifndef MEMORYHEAPOCCUPATION
#define MEMORYHEAPOCCUPATION

#include <vector>

#include "MemoryBlock.hpp"

struct MemoryHeapOccupation {
    MemoryHeapOccupation(size_t pageCount) : blocks(pageCount) {}
    std::vector<MemoryBlock> blocks;
};

#endif