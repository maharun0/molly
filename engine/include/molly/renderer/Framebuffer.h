#pragma once

#include <cstdint>
#include <memory>

#include "molly/renderer/Texture.h"

namespace molly {

struct FramebufferSpec {
    uint32_t width = 1280;
    uint32_t height = 720;
};

class Framebuffer {
public:
    explicit Framebuffer(const FramebufferSpec& spec);
    ~Framebuffer();

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    void bind() const;
    void unbind() const;

    void resize(uint32_t width, uint32_t height);

    uint32_t getColorAttachmentRendererId() const;
    const FramebufferSpec& getSpec() const { return m_spec; }

private:
    void invalidate();

    unsigned int m_rendererId = 0;
    unsigned int m_depthAttachment = 0;
    std::shared_ptr<Texture2D> m_colorAttachment;
    FramebufferSpec m_spec;
};

}  // namespace molly
