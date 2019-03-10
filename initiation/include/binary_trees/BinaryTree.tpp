template <typename T>
BinaryTree<T>::BinaryTree(T value) : mValue(value), mLeft(nullptr), mRight(nullptr), mIsLeaf(true) {}

template <typename T>
BinaryTree<T>::BinaryTree(const BinaryTree<T>& other) : mIsLeaf(other.mIsLeaf), mValue(other.mValue) {
    BinaryTree* otherLeft{other.mLeft.get()};
    BinaryTree* otherRight{other.mRight.get()};
    if (otherLeft) {
        mLeft = std::unique_ptr<BinaryTree>(new BinaryTree(*otherLeft));
    } else {
        mLeft = std::unique_ptr<BinaryTree>(nullptr);
    }
    if (otherRight) {
        mRight = std::unique_ptr<BinaryTree>(new BinaryTree(*otherRight));
    } else {
        mRight = std::unique_ptr<BinaryTree>(nullptr);
    }
}

template <typename T>
BinaryTree<T>::BinaryTree(BinaryTree<T>&& other) : mIsLeaf(std::move(other.mIsLeaf)), mValue(std::move(other.mValue)),
    mLeft(std::move(other.mLeft)), mRight(std::move(other.mRight)) {
}

template <typename T>
BinaryTree<T>& BinaryTree<T>::operator=(const BinaryTree<T>& other) {
    mIsLeaf = other.mIsLeaf;
    mValue = other.mValue;
    if (other.mLeft) {
        mLeft = std::unique_ptr<BinaryTree<T>>(new BinaryTree<T>(*other.mLeft));
    } else {
        mLeft = std::unique_ptr<BinaryTree<T>>(nullptr);
    }
    if (other.mRight) {
        mRight = std::unique_ptr<BinaryTree<T>>(new BinaryTree<T>(*other.mRight));
    } else {
        mRight = std::unique_ptr<BinaryTree<T>>(nullptr);
    }
}

template <typename T>
BinaryTree<T>& BinaryTree<T>::operator=(BinaryTree<T>&& other) {
    mIsLeaf = std::move(other.mIsLeaf);
    mValue = std::move(other.mValue);
    mLeft = std::move(other.mLeft);
    mRight = std::move(other.mRight);
}

template <typename T>
void BinaryTree<T>::setLeft(const BinaryTree<T>& left) {
    mLeft = std::unique_ptr<BinaryTree<T>>(new BinaryTree<T>(left));
    mIsLeaf = false;
}

template <typename T>
void BinaryTree<T>::setLeft(BinaryTree<T>&& left) {
    mLeft = std::unique_ptr<BinaryTree<T>>(new BinaryTree<T>(left));
    mIsLeaf = false;
}

template <typename T>
void BinaryTree<T>::setRight(const BinaryTree<T>& right) {
    mRight = std::unique_ptr<BinaryTree<T>>(new BinaryTree<T>(right));
    mIsLeaf = false;
}

template <typename T>
void BinaryTree<T>::setRight(BinaryTree<T>&& right) {
    mRight = std::unique_ptr<BinaryTree<T>>(new BinaryTree<T>(right));
    mIsLeaf = false;
}

template <typename T>
void BinaryTree<T>::removeLeft() {
    mLeft.reset(nullptr);
    mIsLeaf = !(hasLeft() || hasRight());
}

template <typename T>
void BinaryTree<T>::removeRight() {
    mRight.reset(nullptr);
    mIsLeaf = !(hasLeft() || hasRight());
}

template <typename T>
BinaryTree<T>& BinaryTree<T>::getLeft() {
    if (!hasLeft()) {
        throw std::runtime_error("Error ! This tree doesn't have a left subtree");
    }
    return *mLeft;
}

template <typename T>
const BinaryTree<T>& BinaryTree<T>::getLeft() const {
    if (!hasLeft()) {
        throw std::runtime_error("Error ! This tree doesn't have a left subtree");
    }
    return *mLeft;
}

template <typename T>
BinaryTree<T>& BinaryTree<T>::getRight() {
    if (!hasRight()) {
        throw std::runtime_error("Error ! This tree doesn't have a right subtree");
    }
    return *mRight;
}

template <typename T>
const BinaryTree<T>& BinaryTree<T>::getRight() const {
    if (!hasRight()) {
        throw std::runtime_error("Error ! This tree doesn't have a right subtree");
    }
    return *mRight;
}

template <typename T>
bool BinaryTree<T>::hasLeft() const {
    return (bool)mLeft;
}

template <typename T>
bool BinaryTree<T>::hasRight() const {
    return (bool)mRight;
}

template <typename T>
size_t BinaryTree<T>::getDepth() const {
    if (mIsLeaf) {
        return 0;
    }

    size_t mLeftDepth{0}, mRightDepth{0};
    if (mLeft) {
        mLeftDepth = mLeft->getDepth();
    }

    if (mRight) {
        mRightDepth = mRight->getDepth();
    }
    return 1 + std::max(mLeftDepth, mRightDepth);
}

template <typename T>
size_t BinaryTree<T>::getLeafCount() const {
    if (mIsLeaf) {
        return 1;
    }
    size_t mLeftLeafCount{0}, mRightLeafCount{0};
    if (mLeft) {
        mLeftLeafCount = mLeft->getLeafCount();
    }
    if (mRight) {
        mRightLeafCount = mRight->getLeafCount();
    }
    return mLeftLeafCount + mRightLeafCount;
}

template <typename T>
size_t BinaryTree<T>::getNodeCount() const {
    if (mIsLeaf) {
        return 1;
    }
    size_t mLeftLeafCount{0}, mRightLeafCount{0};
    if (mLeft) {
        mLeftLeafCount = mLeft->getNodeCount();
    }
    if (mRight) {
        mRightLeafCount = mRight->getNodeCount();
    }
    return 1 + mLeftLeafCount + mRightLeafCount;
}

template <typename T>
T& BinaryTree<T>::getValue() {
    return mValue;
}

template <typename T>
const T& BinaryTree<T>::getValue() const {
    return mValue;
}

template <typename T>
bool BinaryTree<T>::isLeaf() const {
    return mIsLeaf;
}