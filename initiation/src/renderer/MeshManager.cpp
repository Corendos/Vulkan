#include "renderer/MeshManager.hpp"

MeshManager::MeshManager() {
    mMeshes.reserve(MaximumMeshCount);
}

void MeshManager::addMesh(Mesh& mesh) {
    assert(mMeshes.size() < MaximumMeshCount);
    mMeshes.push_back(&mesh);
}

void MeshManager::removeMesh(Mesh& mesh) {
    auto it = std::find(mMeshes.begin(), mMeshes.end(), &mesh);
    assert(it != mMeshes.end());
    mMeshes.erase(it);
}