#include "molly/renderer/Texture.h"

#include <glad/gl.h>
#include <stb_image.h>

#include "molly/core/Log.h"

namespace molly {

Texture2D::Texture2D(const std::string& path) : m_path(path) {
    stbi_set_flip_vertically_on_load(1);

    int width = 0, height = 0, channels = 0;
    stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (!data) {
        MOLLY_ERROR("Failed to load texture: {0}", path);
        return;
    }

    m_width = static_cast<uint32_t>(width);
    m_height = static_cast<uint32_t>(height);

    if (channels == 4) {
        m_internalFormat = GL_RGBA8;
        m_dataFormat = GL_RGBA;
    } else if (channels == 3) {
        m_internalFormat = GL_RGB8;
        m_dataFormat = GL_RGB;
    } else if (channels == 1) {
        m_internalFormat = GL_R8;
        m_dataFormat = GL_RED;
    } else {
        MOLLY_ERROR("Unsupported texture channel count ({0}) in {1}", channels, path);
        stbi_image_free(data);
        return;
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererId);
    glTextureStorage2D(m_rendererId, 1, m_internalFormat, static_cast<int>(m_width),
                        static_cast<int>(m_height));

    glTextureParameteri(m_rendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_rendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_rendererId, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_rendererId, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTextureSubImage2D(m_rendererId, 0, 0, 0, static_cast<int>(m_width), static_cast<int>(m_height),
                         m_dataFormat, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
    m_loaded = true;
}

Texture2D::Texture2D(uint32_t width, uint32_t height) : m_width(width), m_height(height) {
    m_internalFormat = GL_RGBA8;
    m_dataFormat = GL_RGBA;

    glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererId);
    glTextureStorage2D(m_rendererId, 1, m_internalFormat, static_cast<int>(m_width),
                        static_cast<int>(m_height));

    glTextureParameteri(m_rendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_rendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_rendererId, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_rendererId, GL_TEXTURE_WRAP_T, GL_REPEAT);

    m_loaded = true;
}

Texture2D::~Texture2D() { glDeleteTextures(1, &m_rendererId); }

void Texture2D::bind(uint32_t slot) const { glBindTextureUnit(slot, m_rendererId); }

void Texture2D::setData(const void* data, uint32_t /*size*/) {
    glTextureSubImage2D(m_rendererId, 0, 0, 0, static_cast<int>(m_width), static_cast<int>(m_height),
                         m_dataFormat, GL_UNSIGNED_BYTE, data);
}

}  // namespace molly
