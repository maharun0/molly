#pragma once

#include <cstdint>
#include <string>

namespace molly {

class Texture2D {
public:
    explicit Texture2D(const std::string& path);
    Texture2D(uint32_t width, uint32_t height);  // blank RGBA texture, e.g. framebuffer color attachment source
    ~Texture2D();

    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;

    void bind(uint32_t slot = 0) const;

    void setData(const void* data, uint32_t size);

    uint32_t getWidth() const { return m_width; }
    uint32_t getHeight() const { return m_height; }
    uint32_t getRendererId() const { return m_rendererId; }

    bool isLoaded() const { return m_loaded; }

private:
    std::string m_path;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_rendererId = 0;
    uint32_t m_internalFormat = 0;
    uint32_t m_dataFormat = 0;
    bool m_loaded = false;
};

}  // namespace molly
