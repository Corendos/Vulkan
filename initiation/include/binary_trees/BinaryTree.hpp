#ifndef __BINARY_TREE_HPP__
#define __BINARY_TREE_HPP__

#include <memory>

/**
 * \file BinaryTree.hpp
 * \brief BinaryTree header file
 * \author Corentin G.
 * \date 15/02/2019
 */

template <typename T>
class BinaryTree {
    public:
        BinaryTree(T value);
        BinaryTree(const BinaryTree& other);
        BinaryTree(BinaryTree&& other);
        BinaryTree& operator=(const BinaryTree& other);
        BinaryTree& operator=(BinaryTree&& other);

        void setLeft(const BinaryTree& left);
        void setLeft(BinaryTree&& left);
        void setRight(const BinaryTree& right);
        void setRight(BinaryTree&& right);

        void removeLeft();
        void removeRight();

        BinaryTree& getLeft();
        const BinaryTree& getLeft() const;
        BinaryTree& getRight();
        const BinaryTree& getRight() const;

        bool hasLeft() const;
        bool hasRight() const;

        size_t getDepth() const;
        size_t getLeafCount() const;
        size_t getNodeCount() const;

        T& getValue();
        const T& getValue() const;
        bool isLeaf() const;

    private:
        T mValue;

        bool mIsLeaf;
        std::unique_ptr<BinaryTree> mLeft;
        std::unique_ptr<BinaryTree> mRight;
};

#include "BinaryTree.tpp"

#endif