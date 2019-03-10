#ifndef __BLOCK_HPP__
#define __BLOCK_HPP__

#include <cstddef>
#include <tuple>

struct Block {
    Block(const size_t blockSize = 0, const size_t blockOffset = 0);
    size_t size;
    size_t offset;
    bool free{true};
    bool full{false};

    std::tuple<Block, Block> divide();

    static bool isPowerOfTwo(const size_t n);
};

bool operator==(const Block& b1, const Block& b2);

#endif