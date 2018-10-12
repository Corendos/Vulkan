#ifndef BINARYTREE
#define BINARYTREE

#include <memory>
#include <vector>

template <typename T>
class BinaryTree {
    public:
        BinaryTree() = default;
        BinaryTree(T& value);
        BinaryTree(T&& value);
        BinaryTree(T value, BinaryTree<T>&& left, BinaryTree<T>&& right);
        BinaryTree(T value, BinaryTree<T>& left, BinaryTree<T>& right);
        BinaryTree(BinaryTree<T>& other);
        BinaryTree(BinaryTree<T>&& other);
        BinaryTree operator=(BinaryTree<T>& other);
        BinaryTree operator=(BinaryTree<T>&& other);

        void setLeft(BinaryTree<T>& left);
        void setLeft(BinaryTree<T>&& left);

        void setRight(BinaryTree<T>& right);
        void setRight(BinaryTree<T>&& right);

        std::vector<T> toArray();

    private:
        T mValue;
        std::shared_ptr<BinaryTree<T>> mLeft;
        std::shared_ptr<BinaryTree<T>> mRight;

        void toArray(std::vector<T>& array);
};

template <typename T>
BinaryTree<T>::BinaryTree(T& value) {
    mValue = value;
}

template <typename T>
BinaryTree<T>::BinaryTree(T&& value) {
    mValue = std::move(value);
}

template <typename T>
BinaryTree<T>::BinaryTree(T value, BinaryTree<T>&& left, BinaryTree<T>&& right) {
    mLeft = std::make_shared<BinaryTree<T>>(std::move(left));
    mRight = std::make_shared<BinaryTree<T>>(std::move(right));
    mValue = value;
}

template <typename T>
BinaryTree<T>::BinaryTree(T value, BinaryTree<T>& left, BinaryTree<T>& right) {
    mLeft = std::make_shared<BinaryTree<T>>(left);
    mRight = std::make_shared<BinaryTree<T>>(right);
    mValue = value;
}

template <typename T>
BinaryTree<T>::BinaryTree(BinaryTree<T>& other) {
    mLeft = other.mLeft;
    mRight = other.mRight;
    mValue = other.mValue;
}

template <typename T>
BinaryTree<T>::BinaryTree(BinaryTree<T>&& other) {
    mLeft = std::move(other.mLeft);
    mRight = std::move(other.mRight);
    mValue = std::move(other.mValue);
}

template <typename T>
BinaryTree<T> BinaryTree<T>::operator=(BinaryTree<T>& other) {
    mLeft = other.mLeft;
    mRight = other.mRight;
    mValue = other.mValue;
}

template <typename T>
BinaryTree<T> BinaryTree<T>::operator=(BinaryTree<T>&& other) {
    mLeft = std::move(other.mLeft);
    mRight = std::move(other.mRight);
    mValue = std::move(other.mValue);
}

template <typename T>
void BinaryTree<T>::setLeft(BinaryTree<T>& left) {
    mLeft = std::make_shared<BinaryTree<T>>(left);
}

template <typename T>
void BinaryTree<T>::setLeft(BinaryTree<T>&& left) {
    mLeft = std::make_shared<BinaryTree<T>>(std::move(left));
}

template <typename T>
void BinaryTree<T>::setRight(BinaryTree<T>& right) {
    mRight = std::make_shared<BinaryTree<T>>(right);
}

template <typename T>
void BinaryTree<T>::setRight(BinaryTree<T>&& right) {
    mRight = std::make_shared<BinaryTree<T>>(std::move(right));
}

template <typename T>
std::vector<T> BinaryTree<T>::toArray() {
    std::vector<T> array;
    array.push_back(mValue);
    if (mLeft)
        mLeft->toArray(array);
    if (mRight)
        mRight->toArray(array);
    return array;
}

template <typename T>
void BinaryTree<T>::toArray(std::vector<T>& array) {
    array.push_back(mValue);
    if (mLeft)
        mLeft->toArray(array);
    if (mRight)
        mRight->toArray(array);
}

#endif