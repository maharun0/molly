#pragma once

#include <memory>
#include <vector>

#include "molly/renderer/Buffer.h"

namespace molly {

class VertexArray {
public:
    VertexArray();
    ~VertexArray();

    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;

    void bind() const;
    void unbind() const;

    void addVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer);
    void setIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer);

    const std::shared_ptr<IndexBuffer>& getIndexBuffer() const { return m_indexBuffer; }

private:
    unsigned int m_rendererId = 0;
    uint32_t m_vertexBufferIndex = 0;
    std::vector<std::shared_ptr<VertexBuffer>> m_vertexBuffers;
    std::shared_ptr<IndexBuffer> m_indexBuffer;
};

}  // namespace molly
