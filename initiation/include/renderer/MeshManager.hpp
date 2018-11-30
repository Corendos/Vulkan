#ifndef MESHMANAGER
#define MESHMANAGER

#include <vector>

#include "renderer/Mesh.hpp"

class MeshManager {
    public:
        MeshManager();
        MeshManager(const MeshManager& other) = delete;
        MeshManager(const MeshManager&& other) = delete;

        void operator=(const MeshManager& other) = delete;
        void operator=(const MeshManager&& other) = delete;

        void addMesh(Mesh& mesh);
        void removeMesh(Mesh& mesh);

        static constexpr size_t MaximumMeshCount{1024};
    private:
        std::vector<Mesh*> mMeshes;
};

#endif