#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "molly/renderer/VertexArray.h"

namespace molly {

struct MeshVertex {
    glm::vec3 position{0.0f};
    glm::vec3 normal{0.0f};
    glm::vec2 texCoord{0.0f};
};

class Mesh {
public:
    Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices);

    void bind() const;
    uint32_t getIndexCount() const { return m_indexCount; }

    static std::shared_ptr<Mesh> createCube(float size = 1.0f);
    static std::shared_ptr<Mesh> createPlane(float size = 1.0f);

private:
    std::shared_ptr<VertexArray> m_vertexArray;
    uint32_t m_indexCount = 0;
};

}  // namespace molly
