#include "EditorLayer.h"

#include <glad/gl.h>
#include <imgui.h>

#include <GLFW/glfw3.h>

namespace molly {

EditorLayer::EditorLayer() : Layer("EditorLayer") {}

void EditorLayer::onAttach() {
    glEnable(GL_DEPTH_TEST);

    auto shader = Shader::createFromFiles("assets/shaders/lit.vert", "assets/shaders/lit.frag");
    m_material = std::make_shared<Material>();
    m_material->shader = shader;
    m_material->albedoColor = {0.8f, 0.8f, 0.85f};

    m_meshes = ModelLoader::load("assets/models/box/Box.gltf");
    if (m_meshes.empty()) {
        MOLLY_WARN("No meshes loaded for the Milestone 2 test model; falling back to a procedural cube");
        m_meshes.push_back(Mesh::createCube());
    }

    m_dirLight.type = LightType::Directional;
    m_dirLight.direction = glm::normalize(glm::vec3(-0.4f, -0.9f, -0.3f));
    m_dirLight.color = {1.0f, 0.96f, 0.9f};
    m_dirLight.intensity = 1.1f;

    m_camera = PerspectiveCamera(60.0f, m_viewportSize.x / m_viewportSize.y, 0.1f, 100.0f);
    m_camera.setPosition({2.5f, 2.5f, 2.5f});
    m_camera.setPitchYaw(-35.0f, -135.0f);

    FramebufferSpec spec;
    spec.width = static_cast<uint32_t>(m_viewportSize.x);
    spec.height = static_cast<uint32_t>(m_viewportSize.y);
    m_framebuffer = std::make_shared<Framebuffer>(spec);
}

void EditorLayer::updateEditorCamera(float dt) {
    auto* window = Application::get().getWindow().getNativeWindow();
    static bool firstMove = true;

    if (!m_viewportFocused || glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS) {
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
    m_camera.setPitchYaw(m_camera.getPitch() - dy * sensitivity,
                          m_camera.getYaw() + dx * sensitivity);

    float speed = 3.0f * dt;
    glm::vec3 pos = m_camera.getPosition();
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) pos += m_camera.getForward() * speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) pos -= m_camera.getForward() * speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) pos -= m_camera.getRight() * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) pos += m_camera.getRight() * speed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) pos += glm::vec3(0.0f, 1.0f, 0.0f) * speed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) pos -= glm::vec3(0.0f, 1.0f, 0.0f) * speed;
    m_camera.setPosition(pos);
}

void EditorLayer::renderScene() {
    m_framebuffer->bind();
    glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_material->bind();
    m_material->shader->setMat4("u_ViewProjection", m_camera.getViewProjectionMatrix());
    m_material->shader->setFloat3("u_ViewPos", m_camera.getPosition());

    glm::mat4 model(1.0f);
    m_material->shader->setMat4("u_Model", model);
    m_material->shader->setMat3("u_NormalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));

    m_material->shader->setFloat3("u_DirLight.direction", m_dirLight.direction);
    m_material->shader->setFloat3("u_DirLight.color", m_dirLight.color);
    m_material->shader->setFloat("u_DirLight.intensity", m_dirLight.intensity);
    m_material->shader->setInt("u_PointLightCount", 0);

    for (const auto& mesh : m_meshes) {
        mesh->bind();
        glDrawElements(GL_TRIANGLES, static_cast<int>(mesh->getIndexCount()), GL_UNSIGNED_INT,
                       nullptr);
    }

    m_framebuffer->unbind();
}

void EditorLayer::onUpdate(float dt) {
    updateEditorCamera(dt);
    renderScene();
}

void EditorLayer::onImGuiRender() {
    ImGui::Begin("Molly Editor - Milestone 2");
    ImGui::Text("Renderer core: textured/lit model, perspective camera, framebuffer-to-ImGui.");
    ImGui::Text("Frame time: %.3f ms (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    ImGui::Text("Hold Right Mouse in the Viewport to fly: WASD + QE, mouse-look.");
    ImGui::End();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport");
    m_viewportFocused = ImGui::IsWindowFocused() || ImGui::IsWindowHovered();

    ImVec2 size = ImGui::GetContentRegionAvail();
    if (size.x > 0 && size.y > 0 &&
        (static_cast<uint32_t>(size.x) != m_framebuffer->getSpec().width ||
         static_cast<uint32_t>(size.y) != m_framebuffer->getSpec().height)) {
        m_framebuffer->resize(static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y));
        m_viewportSize = {size.x, size.y};
        m_camera.setAspectRatio(size.x / size.y);
    }

    ImGui::Image(static_cast<ImTextureID>(static_cast<intptr_t>(m_framebuffer->getColorAttachmentRendererId())),
                 size, ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();
    ImGui::PopStyleVar();
}

}  // namespace molly
