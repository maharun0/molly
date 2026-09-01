#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace molly {

enum class ShaderDataType { None = 0, Float, Float2, Float3, Float4, Int, Int2, Int3, Int4 };

inline uint32_t shaderDataTypeSize(ShaderDataType type) {
    switch (type) {
        case ShaderDataType::Float:  return 4;
        case ShaderDataType::Float2: return 4 * 2;
        case ShaderDataType::Float3: return 4 * 3;
        case ShaderDataType::Float4: return 4 * 4;
        case ShaderDataType::Int:    return 4;
        case ShaderDataType::Int2:   return 4 * 2;
        case ShaderDataType::Int3:   return 4 * 3;
        case ShaderDataType::Int4:   return 4 * 4;
        default: return 0;
    }
}

struct BufferElement {
    std::string name;
    ShaderDataType type;
    uint32_t size;
    size_t offset = 0;
    bool normalized = false;

    BufferElement(ShaderDataType t, std::string n, bool norm = false)
        : name(std::move(n)), type(t), size(shaderDataTypeSize(t)), normalized(norm) {}

    uint32_t getComponentCount() const {
        switch (type) {
            case ShaderDataType::Float:  return 1;
            case ShaderDataType::Float2: return 2;
            case ShaderDataType::Float3: return 3;
            case ShaderDataType::Float4: return 4;
            case ShaderDataType::Int:    return 1;
            case ShaderDataType::Int2:   return 2;
            case ShaderDataType::Int3:   return 3;
            case ShaderDataType::Int4:   return 4;
            default: return 0;
        }
    }
};

class BufferLayout {
public:
    BufferLayout() = default;
    BufferLayout(std::initializer_list<BufferElement> elements) : m_elements(elements) {
        size_t offset = 0;
        m_stride = 0;
        for (auto& element : m_elements) {
            element.offset = offset;
            offset += element.size;
            m_stride += element.size;
        }
    }

    uint32_t getStride() const { return m_stride; }
    const std::vector<BufferElement>& getElements() const { return m_elements; }

    std::vector<BufferElement>::iterator begin() { return m_elements.begin(); }
    std::vector<BufferElement>::iterator end() { return m_elements.end(); }
    std::vector<BufferElement>::const_iterator begin() const { return m_elements.begin(); }
    std::vector<BufferElement>::const_iterator end() const { return m_elements.end(); }

private:
    std::vector<BufferElement> m_elements;
    uint32_t m_stride = 0;
};

class VertexBuffer {
public:
    VertexBuffer(const float* vertices, uint32_t size);
    ~VertexBuffer();

    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    void bind() const;
    void unbind() const;

    const BufferLayout& getLayout() const { return m_layout; }
    void setLayout(const BufferLayout& layout) { m_layout = layout; }

private:
    unsigned int m_rendererId = 0;
    BufferLayout m_layout;
};

class IndexBuffer {
public:
    IndexBuffer(const uint32_t* indices, uint32_t count);
    ~IndexBuffer();

    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;

    void bind() const;
    void unbind() const;

    uint32_t getCount() const { return m_count; }

private:
    unsigned int m_rendererId = 0;
    uint32_t m_count;
};

}  // namespace molly
