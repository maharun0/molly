#include "ViewportPanel.h"

#include <glad/gl.h>
#include <imgui.h>

#include <GLFW/glfw3.h>

namespace molly {

ViewportPanel::ViewportPanel() {
    m_editorCamera = PerspectiveCamera(60.0f, m_viewportSize.x / m_viewportSize.y, 0.1f, 100.0f);
    m_editorCamera.setPosition({2.5f, 2.5f, 2.5f});
    m_editorCamera.setPitchYaw(-35.0f, -135.0f);

    FramebufferSpec spec;
    spec.width = static_cast<uint32_t>(m_viewportSize.x);
    spec.height = static_cast<uint32_t>(m_viewportSize.y);
    m_framebuffer = std::make_shared<Framebuffer>(spec);
}

void ViewportPanel::updateEditorCamera(float dt) {
    auto* window = Application::get().getWindow().getNativeWindow();
    static bool firstMove = true;

    if (!m_focused || glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstMove = true;
        return;
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    static double lastX = 0.0, lastY = 0.0;
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    if (firstMove) {
        lastX = x;
        lastY = y;
        firstMove = false;
    }
    float dx = static_cast<float>(x - lastX);
    float dy = static_cast<float>(y - lastY);
    lastX = x;
    lastY = y;

    constexpr float sensitivity = 0.15f;
    m_editorCamera.setPitchYaw(m_editorCamera.getPitch() - dy * sensitivity,
                                m_editorCamera.getYaw() + dx * sensitivity);

    float speed = 3.0f * dt;
    glm::vec3 pos = m_editorCamera.getPosition();
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) pos += m_editorCamera.getForward() * speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) pos -= m_editorCamera.getForward() * speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) pos -= m_editorCamera.getRight() * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) pos += m_editorCamera.getRight() * speed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) pos += glm::vec3(0.0f, 1.0f, 0.0f) * speed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) pos -= glm::vec3(0.0f, 1.0f, 0.0f) * speed;
    m_editorCamera.setPosition(pos);
}

void ViewportPanel::onUpdate(float dt) { updateEditorCamera(dt); }

void ViewportPanel::onImGuiRender(Scene* scene, PerspectiveCamera* overrideCamera) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport");
    m_focused = ImGui::IsWindowFocused() || ImGui::IsWindowHovered();

    ImVec2 size = ImGui::GetContentRegionAvail();
    if (size.x > 0 && size.y > 0 &&
        (static_cast<uint32_t>(size.x) != m_framebuffer->getSpec().width ||
         static_cast<uint32_t>(size.y) != m_framebuffer->getSpec().height)) {
        m_framebuffer->resize(static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y));
        m_viewportSize = {size.x, size.y};
        m_editorCamera.setAspectRatio(size.x / size.y);
    }

    m_framebuffer->bind();
    glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (scene) {
        PerspectiveCamera& camera = overrideCamera ? *overrideCamera : m_editorCamera;
        renderScene(scene->getRegistry(), camera.getViewProjectionMatrix(), camera.getPosition());
    }

    m_framebuffer->unbind();

    ImGui::Image(static_cast<ImTextureID>(static_cast<intptr_t>(m_framebuffer->getColorAttachmentRendererId())),
                 size, ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();
    ImGui::PopStyleVar();
}

}  // namespace molly
