#ifndef IMPORTER
#define IMPORTER

#include <string>
#include <assimp/Importer.hpp>

#include "renderer/mesh/Mesh.hpp"

class Importer {
    public:
        Mesh loadMesh(std::string filename);
    private:
        Assimp::Importer mImporter;
};

#endif