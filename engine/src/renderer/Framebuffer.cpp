#include "molly/renderer/Framebuffer.h"

#include <glad/gl.h>

#include "molly/core/Log.h"

namespace molly {

Framebuffer::Framebuffer(const FramebufferSpec& spec) : m_spec(spec) { invalidate(); }

Framebuffer::~Framebuffer() {
    glDeleteFramebuffers(1, &m_rendererId);
    glDeleteRenderbuffers(1, &m_depthAttachment);
}

void Framebuffer::invalidate() {
    if (m_rendererId) {
        glDeleteFramebuffers(1, &m_rendererId);
        glDeleteRenderbuffers(1, &m_depthAttachment);
    }

    glCreateFramebuffers(1, &m_rendererId);

    m_colorAttachment = std::make_shared<Texture2D>(m_spec.width, m_spec.height);
    glNamedFramebufferTexture(m_rendererId, GL_COLOR_ATTACHMENT0,
                               m_colorAttachment->getRendererId(), 0);

    glCreateRenderbuffers(1, &m_depthAttachment);
    glNamedRenderbufferStorage(m_depthAttachment, GL_DEPTH24_STENCIL8, static_cast<int>(m_spec.width),
                                static_cast<int>(m_spec.height));
    glNamedFramebufferRenderbuffer(m_rendererId, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                    m_depthAttachment);

    if (glCheckNamedFramebufferStatus(m_rendererId, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        MOLLY_ERROR("Framebuffer is incomplete");
    }
}

void Framebuffer::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_rendererId);
    glViewport(0, 0, static_cast<int>(m_spec.width), static_cast<int>(m_spec.height));
}

void Framebuffer::unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void Framebuffer::resize(uint32_t width, uint32_t height) {
    if (width == 0 || height == 0) return;
    m_spec.width = width;
    m_spec.height = height;
    invalidate();
}

uint32_t Framebuffer::getColorAttachmentRendererId() const {
    return m_colorAttachment->getRendererId();
}

}  // namespace molly
