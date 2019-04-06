#include "memory/Chunk.hpp"

#include <cmath>

size_t Chunk::MinBlockSize = 4 * 1024;

Chunk::Chunk(vk::DeviceMemory memory, const size_t chunkSize, const size_t minBlockSize) : mTree(Block(chunkSize)), mMinBlockSize(minBlockSize), mMemory(memory) {
}

AllocationResult Chunk::reserve(const size_t blockSize) {
    int nextPowerOfTwo = (size_t)std::ceil(std::log2(blockSize));
    int realBlockSize = (1 << nextPowerOfTwo) < mMinBlockSize ? mMinBlockSize : (1 << nextPowerOfTwo);

    AllocationResult result = _reserve(realBlockSize, mTree);

    return result;
}

AllocationResult Chunk::_reserve(const size_t blockSize, BlockBinaryTree& memory) {
    Block& currentBlock = memory.getValue();
    AllocationResult result;

    if (!currentBlock.free) {
        result.found = false;
        return result;
    } else {
        if (memory.isLeaf()) {
            if (currentBlock.size < blockSize) {
                result.found = false;
                return result;
            } else if (currentBlock.size == blockSize) {
                currentBlock.free = false;

                result.found = true;
                result.block = currentBlock;
                return result;
            } else {
                Block leftBlock, rightBlock;
                std::tie(leftBlock, rightBlock) = currentBlock.divide();
                memory.setLeft(BlockBinaryTree(leftBlock));
                memory.setRight(BlockBinaryTree(rightBlock));
                AllocationResult subResult = _reserve(blockSize, memory.getLeft());
                result.found = subResult.found;
                result.block = subResult.block;
                return result;
            }
        } else {
            if (currentBlock.size <= blockSize) {
                result.found = false;
                return result;
            } else {
                BlockBinaryTree* searchSubTree = &memory.getLeft();
                BlockBinaryTree* otherSubTree = &memory.getRight();
                AllocationResult subResult = _reserve(blockSize, *searchSubTree);

                if (!subResult.found) {
                    searchSubTree = &memory.getRight();
                    otherSubTree = &memory.getLeft();
                    subResult = _reserve(blockSize, *searchSubTree);
                }

                if (!subResult.found) {
                    result.found = false;
                    return result;
                } else {
                    currentBlock.free = memory.getLeft().getValue().free || memory.getRight().getValue().free;

                    result.found = true;
                    result.block = subResult.block;
                    return result;
                }
            }
        }
        throw std::runtime_error("This should not happen");
    }
}

bool Chunk::free(Block block) {
    bool result = _free(block, mTree);
    return result;
}

const vk::DeviceMemory Chunk::getMemory() const {
    return mMemory;
}

BlockBinaryTree& Chunk::getTree() {
    return mTree;
}

void Chunk::setMinBlockSize(const size_t minBlockSize) {
    mMinBlockSize = minBlockSize;
}

bool Chunk::_free(Block block, BlockBinaryTree& memory) {
    Block& currentBlock = memory.getValue();
    bool result;
    if (currentBlock == block) {
        currentBlock.free = true;

        result = true;
        return result;
    } else if (currentBlock.size < block.size) {
        result = false;
        return result;
    } else {
        if (memory.isLeaf()) {
            result = false;
            return result;
        } else {
            bool resultLeft, resultRight;
            resultLeft = _free(block, memory.getLeft());
            resultRight = _free(block, memory.getRight());

            currentBlock.free = memory.getLeft().getValue().free || memory.getRight().getValue().free;

            if (memory.getLeft().isLeaf() && memory.getLeft().getValue().free &&
                memory.getRight().isLeaf() && memory.getRight().getValue().free) {
                memory.removeLeft();
                memory.removeRight();
            }

            result = resultLeft || resultRight;

            return result;
        }
    }
}