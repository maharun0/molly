#include "molly/renderer/VertexArray.h"

#include <glad/gl.h>

#include "molly/core/Assert.h"

namespace molly {

static GLenum toGLBaseType(ShaderDataType type) {
    switch (type) {
        case ShaderDataType::Float:
        case ShaderDataType::Float2:
        case ShaderDataType::Float3:
        case ShaderDataType::Float4:
            return GL_FLOAT;
        case ShaderDataType::Int:
        case ShaderDataType::Int2:
        case ShaderDataType::Int3:
        case ShaderDataType::Int4:
            return GL_INT;
        default:
            return 0;
    }
}

VertexArray::VertexArray() { glCreateVertexArrays(1, &m_rendererId); }

VertexArray::~VertexArray() { glDeleteVertexArrays(1, &m_rendererId); }

void VertexArray::bind() const { glBindVertexArray(m_rendererId); }

void VertexArray::unbind() const { glBindVertexArray(0); }

void VertexArray::addVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) {
    MOLLY_ASSERT(!vertexBuffer->getLayout().getElements().empty(), "Vertex buffer has no layout");

    glBindVertexArray(m_rendererId);
    vertexBuffer->bind();

    const auto& layout = vertexBuffer->getLayout();
    for (const auto& element : layout) {
        glEnableVertexAttribArray(m_vertexBufferIndex);
        glVertexAttribPointer(m_vertexBufferIndex, static_cast<int>(element.getComponentCount()),
                               toGLBaseType(element.type), element.normalized ? GL_TRUE : GL_FALSE,
                               static_cast<int>(layout.getStride()),
                               reinterpret_cast<const void*>(element.offset));
        ++m_vertexBufferIndex;
    }

    m_vertexBuffers.push_back(vertexBuffer);
}

void VertexArray::setIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) {
    glBindVertexArray(m_rendererId);
    indexBuffer->bind();
    m_indexBuffer = indexBuffer;
}

}  // namespace molly
