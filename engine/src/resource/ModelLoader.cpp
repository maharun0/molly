#include "molly/resource/ModelLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "molly/core/Log.h"

namespace molly {

static std::shared_ptr<Mesh> processMesh(const aiMesh* mesh) {
    std::vector<MeshVertex> vertices;
    vertices.reserve(mesh->mNumVertices);

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        MeshVertex vertex;
        vertex.position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};

        if (mesh->HasNormals()) {
            vertex.normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
        }

        if (mesh->mTextureCoords[0]) {
            vertex.texCoord = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
        }

        vertices.push_back(vertex);
    }

    std::vector<uint32_t> indices;
    indices.reserve(static_cast<size_t>(mesh->mNumFaces) * 3);
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        const aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    return std::make_shared<Mesh>(vertices, indices);
}

std::vector<std::shared_ptr<Mesh>> ModelLoader::load(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs |
                  aiProcess_JoinIdenticalVertices);

    std::vector<std::shared_ptr<Mesh>> meshes;

    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
        MOLLY_ERROR("Assimp failed to load model \"{0}\": {1}", path, importer.GetErrorString());
        return meshes;
    }

    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        auto mesh = processMesh(scene->mMeshes[i]);
        mesh->setDescriptor({MeshSource::Model, path, static_cast<int>(i), 1.0f});
        meshes.push_back(mesh);
    }

    MOLLY_INFO("Loaded model \"{0}\" ({1} mesh(es))", path, meshes.size());
    return meshes;
}

}  // namespace molly
