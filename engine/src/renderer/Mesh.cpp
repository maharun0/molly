#include "molly/renderer/Mesh.h"

namespace molly {

Mesh::Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices) {
    m_indexCount = static_cast<uint32_t>(indices.size());

    auto vb = std::make_shared<VertexBuffer>(
        reinterpret_cast<const float*>(vertices.data()),
        static_cast<uint32_t>(vertices.size() * sizeof(MeshVertex)));
    vb->setLayout({
        {ShaderDataType::Float3, "a_Position"},
        {ShaderDataType::Float3, "a_Normal"},
        {ShaderDataType::Float2, "a_TexCoord"},
    });

    auto ib = std::make_shared<IndexBuffer>(indices.data(), static_cast<uint32_t>(indices.size()));

    m_vertexArray = std::make_shared<VertexArray>();
    m_vertexArray->addVertexBuffer(vb);
    m_vertexArray->setIndexBuffer(ib);
}

void Mesh::bind() const { m_vertexArray->bind(); }

std::shared_ptr<Mesh> Mesh::createCube(float size) {
    float h = size * 0.5f;

    // clang-format off
    std::vector<MeshVertex> vertices = {
        // +Z
        {{-h, -h,  h}, {0, 0, 1}, {0, 0}}, {{ h, -h,  h}, {0, 0, 1}, {1, 0}},
        {{ h,  h,  h}, {0, 0, 1}, {1, 1}}, {{-h,  h,  h}, {0, 0, 1}, {0, 1}},
        // -Z
        {{ h, -h, -h}, {0, 0, -1}, {0, 0}}, {{-h, -h, -h}, {0, 0, -1}, {1, 0}},
        {{-h,  h, -h}, {0, 0, -1}, {1, 1}}, {{ h,  h, -h}, {0, 0, -1}, {0, 1}},
        // +X
        {{ h, -h,  h}, {1, 0, 0}, {0, 0}}, {{ h, -h, -h}, {1, 0, 0}, {1, 0}},
        {{ h,  h, -h}, {1, 0, 0}, {1, 1}}, {{ h,  h,  h}, {1, 0, 0}, {0, 1}},
        // -X
        {{-h, -h, -h}, {-1, 0, 0}, {0, 0}}, {{-h, -h,  h}, {-1, 0, 0}, {1, 0}},
        {{-h,  h,  h}, {-1, 0, 0}, {1, 1}}, {{-h,  h, -h}, {-1, 0, 0}, {0, 1}},
        // +Y
        {{-h,  h,  h}, {0, 1, 0}, {0, 0}}, {{ h,  h,  h}, {0, 1, 0}, {1, 0}},
        {{ h,  h, -h}, {0, 1, 0}, {1, 1}}, {{-h,  h, -h}, {0, 1, 0}, {0, 1}},
        // -Y
        {{-h, -h, -h}, {0, -1, 0}, {0, 0}}, {{ h, -h, -h}, {0, -1, 0}, {1, 0}},
        {{ h, -h,  h}, {0, -1, 0}, {1, 1}}, {{-h, -h,  h}, {0, -1, 0}, {0, 1}},
    };
    // clang-format on

    std::vector<uint32_t> indices;
    for (uint32_t face = 0; face < 6; ++face) {
        uint32_t base = face * 4;
        indices.insert(indices.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
    }

    auto mesh = std::make_shared<Mesh>(vertices, indices);
    mesh->m_descriptor = {MeshSource::Cube, "", 0, size};
    return mesh;
}

std::shared_ptr<Mesh> Mesh::createPlane(float size) {
    float h = size * 0.5f;
    std::vector<MeshVertex> vertices = {
        {{-h, 0.0f, -h}, {0, 1, 0}, {0, 0}},
        {{ h, 0.0f, -h}, {0, 1, 0}, {1, 0}},
        {{ h, 0.0f,  h}, {0, 1, 0}, {1, 1}},
        {{-h, 0.0f,  h}, {0, 1, 0}, {0, 1}},
    };
    std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};
    auto mesh = std::make_shared<Mesh>(vertices, indices);
    mesh->m_descriptor = {MeshSource::Plane, "", 0, size};
    return mesh;
}

}  // namespace molly
