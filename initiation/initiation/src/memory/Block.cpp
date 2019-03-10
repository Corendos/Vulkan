#include "memory/Block.hpp"

Block::Block(const size_t blockSize, const size_t blockOffset) {
    if (!isPowerOfTwo(blockSize)) {
        throw std::runtime_error("Error ! 'blockSize' must be a power of two.");
    }
    size = blockSize;
    offset = blockOffset;
}

std::tuple<Block, Block> Block::divide() {
    size_t newSize = size >> 1;
    return std::make_tuple<Block, Block>({newSize, offset}, {newSize, offset + newSize});
}

bool Block::isPowerOfTwo(const size_t n) {
    return (n & (n - 1)) == 0;
}

bool operator==(const Block& b1, const Block& b2) {
    return (b1.size == b2.size) && (b1.offset == b2.offset);
}