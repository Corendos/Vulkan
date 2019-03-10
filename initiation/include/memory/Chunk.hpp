#ifndef __CHUNK_HPP__
#define __CHUNK_HPP__

#include <vulkan/vulkan.h>

#include "memory/Block.hpp"
#include "binary_trees/BinaryTree.hpp"

struct AllocationResult {
    bool found;
    Block block;
};

using BlockBinaryTree = BinaryTree<Block>;

class Chunk {
    public:
        Chunk(VkDeviceMemory memory, const size_t chunkSize, const size_t minBlockSize = MinBlockSize);
        AllocationResult reserve(const size_t blockSize);
        bool free(Block block);

        const VkDeviceMemory getMemory() const;
        BlockBinaryTree& getTree();

        void setMinBlockSize(const size_t minBlockSize);

        static size_t MinBlockSize;

    private:
        BlockBinaryTree mTree;
        size_t mMinBlockSize;
        VkDeviceMemory mMemory;

        static AllocationResult _reserve(const size_t blockSize, BlockBinaryTree& memory);
        static bool _free(Block block, BlockBinaryTree& memory);
};

#endif