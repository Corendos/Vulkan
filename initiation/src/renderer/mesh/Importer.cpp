#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "renderer/mesh/Importer.hpp"
#include "environment.hpp"

Mesh Importer::loadMesh(std::string filename) {
    const aiScene* scene = mImporter.ReadFile(std::string(ROOT_PATH) + std::string("meshes/") + filename,
        aiProcess_Triangulate);
    
    assert(scene != nullptr);
    
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    for (size_t i{0};i < scene->mMeshes[0]->mNumVertices;i++) {
        Vertex v;
        v.pos.x = scene->mMeshes[0]->mVertices[i].x;
        v.pos.y = scene->mMeshes[0]->mVertices[i].y;
        v.pos.z = scene->mMeshes[0]->mVertices[i].z;

        v.normals.x = scene->mMeshes[0]->mNormals[i].x;
        v.normals.y = scene->mMeshes[0]->mNormals[i].y;
        v.normals.z = scene->mMeshes[0]->mNormals[i].z;

        v.texCoord.x = scene->mMeshes[0]->mTextureCoords[0][i].x;
        v.texCoord.y = 1.0 - scene->mMeshes[0]->mTextureCoords[0][i].y;
        vertices.push_back(v);
    }
    for (size_t i{0};i < scene->mMeshes[0]->mNumFaces;i++) {
        indices.push_back(scene->mMeshes[0]->mFaces[i].mIndices[0]);
        indices.push_back(scene->mMeshes[0]->mFaces[i].mIndices[1]);
        indices.push_back(scene->mMeshes[0]->mFaces[i].mIndices[2]);
    }
    return Mesh(std::move(vertices), std::move(indices));
}
