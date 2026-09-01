#include "molly/renderer/Texture.h"

#include <glad/gl.h>
#include <stb_image.h>

#include "molly/core/Log.h"

namespace molly {

// Loads pixel data from `path` into a brand-new GL texture object, deleting
// whatever the object previously owned first. Returns false (leaving the
// previous texture, if any, untouched) on failure — used by both the
// constructor and reload().
static bool uploadFromFile(const std::string& path, uint32_t& outWidth, uint32_t& outHeight,
                            uint32_t& outInternalFormat, uint32_t& outDataFormat,
                            unsigned int& inOutRendererId) {
    stbi_set_flip_vertically_on_load(1);

    int width = 0, height = 0, channels = 0;
    stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (!data) {
        MOLLY_ERROR("Failed to load texture: {0}", path);
        return false;
    }

    uint32_t internalFormat = 0, dataFormat = 0;
    if (channels == 4) {
        internalFormat = GL_RGBA8;
        dataFormat = GL_RGBA;
    } else if (channels == 3) {
        internalFormat = GL_RGB8;
        dataFormat = GL_RGB;
    } else if (channels == 1) {
        internalFormat = GL_R8;
        dataFormat = GL_RED;
    } else {
        MOLLY_ERROR("Unsupported texture channel count ({0}) in {1}", channels, path);
        stbi_image_free(data);
        return false;
    }

    if (inOutRendererId) glDeleteTextures(1, &inOutRendererId);
    glCreateTextures(GL_TEXTURE_2D, 1, &inOutRendererId);
    glTextureStorage2D(inOutRendererId, 1, internalFormat, width, height);

    glTextureParameteri(inOutRendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(inOutRendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(inOutRendererId, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(inOutRendererId, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTextureSubImage2D(inOutRendererId, 0, 0, 0, width, height, dataFormat, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    outWidth = static_cast<uint32_t>(width);
    outHeight = static_cast<uint32_t>(height);
    outInternalFormat = internalFormat;
    outDataFormat = dataFormat;
    return true;
}

Texture2D::Texture2D(const std::string& path) : m_path(path) {
    m_loaded = uploadFromFile(path, m_width, m_height, m_internalFormat, m_dataFormat, m_rendererId);
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

bool Texture2D::reload() {
    if (m_path.empty()) return false;

    uint32_t width = 0, height = 0, internalFormat = 0, dataFormat = 0;
    unsigned int rendererId = m_rendererId;
    if (!uploadFromFile(m_path, width, height, internalFormat, dataFormat, rendererId)) {
        MOLLY_ERROR("Texture reload failed for \"{0}\" — keeping previous version", m_path);
        return false;
    }

    m_width = width;
    m_height = height;
    m_internalFormat = internalFormat;
    m_dataFormat = dataFormat;
    m_rendererId = rendererId;
    return true;
}

}  // namespace molly
