#include "ViewportPanel.h"

#include <cstdio>

#include <glad/gl.h>
#include <imgui.h>

#include <GLFW/glfw3.h>

namespace molly {

namespace {
// Deliberately not the theme's accent blue — an outline needs to read
// clearly against arbitrary object/material colors, including blue ones.
constexpr const char* kOutlineVertSrc = R"(
#version 460 core
layout(location = 0) in vec3 a_Position;
uniform mat4 u_ViewProjection;
uniform mat4 u_Model;
void main() {
    gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
}
)";
constexpr const char* kOutlineFragSrc = R"(
#version 460 core
out vec4 o_Color;
uniform vec3 u_Color;
void main() {
    o_Color = vec4(u_Color, 1.0);
}
)";
}  // namespace

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

void ViewportPanel::frameEntity(entt::registry& registry, entt::entity entity) {
    if (!registry.valid(entity)) return;

    glm::mat4 world = getWorldTransform(registry, entity);
    glm::vec3 targetPos(world[3]);

    // No per-mesh bounds are tracked, so a fixed distance is used rather
    // than fitting to the object's actual size — good enough to bring an
    // off-screen selection back into view without needing bounding-box
    // plumbing through Mesh/MeshRendererComponent.
    constexpr float kFrameDistance = 4.0f;
    m_editorCamera.setPosition(targetPos - m_editorCamera.getForward() * kFrameDistance);
}

void ViewportPanel::drawWireframeMesh(Mesh& mesh, const glm::mat4& model,
                                       const glm::mat4& viewProjection, const glm::vec3& color) {
    if (!m_outlineShader) {
        m_outlineShader = std::make_shared<Shader>(kOutlineVertSrc, kOutlineFragSrc);
    }

    GLint prevPolygonMode[2];
    glGetIntegerv(GL_POLYGON_MODE, prevPolygonMode);

    m_outlineShader->bind();
    m_outlineShader->setMat4("u_ViewProjection", viewProjection);
    m_outlineShader->setMat4("u_Model", model);
    m_outlineShader->setFloat3("u_Color", color);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(2.0f);
    glDepthFunc(GL_LEQUAL);

    mesh.bind();
    glDrawElements(GL_TRIANGLES, static_cast<int>(mesh.getIndexCount()), GL_UNSIGNED_INT, nullptr);

    glPolygonMode(GL_FRONT_AND_BACK, static_cast<GLenum>(prevPolygonMode[0]));
    glLineWidth(1.0f);
    glDepthFunc(GL_LESS);
}

void ViewportPanel::drawSelectionOutline(entt::registry& registry, entt::entity entity,
                                          const glm::mat4& viewProjection) {
    if (!registry.valid(entity)) return;

    glm::mat4 model = getWorldTransform(registry, entity);

    if (auto* meshRenderer = registry.try_get<MeshRendererComponent>(entity);
        meshRenderer && meshRenderer->mesh) {
        drawWireframeMesh(*meshRenderer->mesh, model, viewProjection, glm::vec3(1.0f, 0.55f, 0.0f));
        return;
    }

    // No mesh (e.g. a light or camera entity) — outline the gizmo marker
    // instead of skipping the selection highlight entirely.
    if (registry.any_of<LightComponent, CameraComponent>(entity)) {
        if (!m_gizmoMesh) m_gizmoMesh = Mesh::createCube(0.3f);
        drawWireframeMesh(*m_gizmoMesh, model, viewProjection, glm::vec3(1.0f, 0.55f, 0.0f));
    }
}

void ViewportPanel::drawEntityGizmos(entt::registry& registry, const glm::mat4& viewProjection) {
    if (!m_gizmoMesh) m_gizmoMesh = Mesh::createCube(0.3f);

    for (auto entity : registry.view<TransformComponent, LightComponent>()) {
        glm::mat4 model = getWorldTransform(registry, entity);
        drawWireframeMesh(*m_gizmoMesh, model, viewProjection, glm::vec3(1.0f, 0.9f, 0.3f));
    }
    for (auto entity : registry.view<TransformComponent, CameraComponent>()) {
        glm::mat4 model = getWorldTransform(registry, entity);
        drawWireframeMesh(*m_gizmoMesh, model, viewProjection, glm::vec3(0.3f, 0.85f, 1.0f));
    }
}

void ViewportPanel::onImGuiRender(Scene* scene, PerspectiveCamera* overrideCamera, Entity selected) {
    if (!isOpen) return;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport", &isOpen);
    m_focused = ImGui::IsWindowFocused() || ImGui::IsWindowHovered();

    // Read here, not in onUpdate() — onUpdate() runs before ImGui::NewFrame()
    // each frame, so io.MouseWheel there is always one frame stale (it's only
    // refreshed by NewFrame draining the queued scroll events). Scroll-wheel
    // dolly zoom works whenever the viewport is hovered, independent of the
    // RMB fly-look mode (matches Unity/Godot/Blender's scene-view zoom).
    if (m_focused && ImGui::GetIO().MouseWheel != 0.0f && overrideCamera == nullptr) {
        constexpr float zoomSpeed = 1.0f;
        glm::vec3 pos = m_editorCamera.getPosition();
        pos += m_editorCamera.getForward() * ImGui::GetIO().MouseWheel * zoomSpeed;
        m_editorCamera.setPosition(pos);
    }

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
        if (!overrideCamera) {
            drawEntityGizmos(scene->getRegistry(), camera.getViewProjectionMatrix());
        }
        if (selected) {
            drawSelectionOutline(scene->getRegistry(), selected, camera.getViewProjectionMatrix());
        }
    }

    m_framebuffer->unbind();

    ImGui::Image(static_cast<ImTextureID>(static_cast<intptr_t>(m_framebuffer->getColorAttachmentRendererId())),
                 size, ImVec2(0, 1), ImVec2(1, 0));

    ImVec2 imageMin = ImGui::GetItemRectMin();
    ImVec2 imageMax = ImGui::GetItemRectMax();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Play-mode border tint (plan/EDITOR_UI_PLAN.md §9.3.1) — a non-null
    // overrideCamera means we're rendering through the runtime scene's own
    // camera, i.e. Playing or Paused, not Edit mode.
    if (overrideCamera) {
        ImU32 borderColor = IM_COL32(61, 139, 255, 255);  // kAccent, Appendix C
        drawList->AddRect(imageMin, imageMax, borderColor, 0.0f, 0, 3.0f);
    }

    char fpsText[32];
    std::snprintf(fpsText, sizeof(fpsText), "%.0f FPS", ImGui::GetIO().Framerate);
    drawList->AddText(ImVec2(imageMin.x + 8, imageMin.y + 6), IM_COL32(255, 255, 255, 200), fpsText);

    if (m_focused && !overrideCamera) {
        const char* hint = "RMB + WASD/QE to fly | Scroll to zoom | F to focus selection";
        drawList->AddText(ImVec2(imageMin.x + 8, imageMin.y + 24), IM_COL32(220, 220, 220, 160), hint);
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

}  // namespace molly
