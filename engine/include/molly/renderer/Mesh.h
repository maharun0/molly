#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "molly/renderer/VertexArray.h"

namespace molly {

struct MeshVertex {
    glm::vec3 position{0.0f};
    glm::vec3 normal{0.0f};
    glm::vec2 texCoord{0.0f};
};

// How a Mesh's geometry was produced — lets SceneSerializer round-trip a
// MeshRendererComponent by re-deriving the same mesh on load (via
// ResourceManager::loadModel for file-sourced meshes, or by calling the
// same createCube/createPlane factory for procedural ones) instead of
// needing to serialize raw vertex data (PLAN.md §14.5, §37).
enum class MeshSource { None, Model, Cube, Plane };

struct MeshDescriptor {
    MeshSource source = MeshSource::None;
    std::string modelPath;  // Model only
    int meshIndex = 0;      // Model only — which sub-mesh in that file
    float size = 1.0f;      // Cube/Plane only
};

class Mesh {
public:
    Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices);

    void bind() const;
    uint32_t getIndexCount() const { return m_indexCount; }

    const MeshDescriptor& getDescriptor() const { return m_descriptor; }
    void setDescriptor(const MeshDescriptor& descriptor) { m_descriptor = descriptor; }

    static std::shared_ptr<Mesh> createCube(float size = 1.0f);
    static std::shared_ptr<Mesh> createPlane(float size = 1.0f);

private:
    std::shared_ptr<VertexArray> m_vertexArray;
    uint32_t m_indexCount = 0;
    MeshDescriptor m_descriptor;
};

}  // namespace molly
