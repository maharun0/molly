#include "EditorLayer.h"

#include <string>

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

    auto floorMaterial = std::make_shared<Material>();
    floorMaterial->shader = shader;
    floorMaterial->albedoColor = {0.35f, 0.38f, 0.42f};

    auto boxMeshes = ModelLoader::load("assets/models/box/Box.gltf");
    std::shared_ptr<Mesh> boxMesh = boxMeshes.empty() ? Mesh::createCube() : boxMeshes[0];

    Entity floor = m_scene.createEntity("Floor");
    floor.getComponent<TransformComponent>().position = {0.0f, -0.6f, 0.0f};
    auto& floorRenderer = floor.addComponent<MeshRendererComponent>();
    floorRenderer.mesh = Mesh::createPlane(10.0f);
    floorRenderer.material = floorMaterial;

    Entity prop = m_scene.createEntity("Prop");
    auto& propRenderer = prop.addComponent<MeshRendererComponent>();
    propRenderer.mesh = boxMesh;
    propRenderer.material = m_material;

    Entity light = m_scene.createEntity("Sun");
    light.getComponent<TransformComponent>().rotation = {-45.0f, 45.0f, 0.0f};
    auto& lightComp = light.addComponent<LightComponent>();
    lightComp.type = LightType::Directional;
    lightComp.color = {1.0f, 0.96f, 0.9f};
    lightComp.intensity = 1.1f;

    m_editorCamera = PerspectiveCamera(60.0f, m_viewportSize.x / m_viewportSize.y, 0.1f, 100.0f);
    m_editorCamera.setPosition({2.5f, 2.5f, 2.5f});
    m_editorCamera.setPitchYaw(-35.0f, -135.0f);

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

void EditorLayer::onUpdate(float dt) {
    updateEditorCamera(dt);
    m_scene.onUpdate(dt);

    m_framebuffer->bind();
    glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderScene(m_scene.getRegistry(), m_editorCamera.getViewProjectionMatrix(),
                m_editorCamera.getPosition());

    m_framebuffer->unbind();
}

void EditorLayer::drawHierarchyPanel() {
    ImGui::Begin("Hierarchy");

    if (ImGui::Button("Create Entity")) {
        ++m_spawnCounter;
        Entity entity = m_scene.createEntity("Entity" + std::to_string(m_spawnCounter));
        auto& renderer = entity.addComponent<MeshRendererComponent>();
        renderer.mesh = Mesh::createCube(0.5f);
        renderer.material = m_material;
        m_selected = entity;
    }

    ImGui::Separator();

    for (auto entity : m_scene.getRegistry().view<TagComponent>()) {
        auto& tag = m_scene.getRegistry().get<TagComponent>(entity);
        bool isSelected = m_selected == Entity(entity, &m_scene);
        if (ImGui::Selectable(tag.name.c_str(), isSelected)) {
            m_selected = Entity(entity, &m_scene);
        }
    }
    ImGui::End();

    ImGui::Begin("Inspector");
    if (m_selected) {
        auto& tag = m_selected.getComponent<TagComponent>();
        ImGui::Text("%s", tag.name.c_str());

        auto& transform = m_selected.getComponent<TransformComponent>();
        ImGui::DragFloat3("Position", &transform.position.x, 0.1f);
        ImGui::DragFloat3("Rotation", &transform.rotation.x, 1.0f);
        ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f);

        if (ImGui::Button("Delete Entity")) {
            m_scene.destroyEntity(m_selected);
            m_selected = {};
        }
    } else {
        ImGui::TextDisabled("No entity selected");
    }
    ImGui::End();
}

void EditorLayer::onImGuiRender() {
    ImGui::Begin("Molly Editor - Milestone 3");
    ImGui::Text("ECS: entities/components drive the scene (entt).");
    ImGui::Text("Frame time: %.3f ms (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    ImGui::Text("Hold Right Mouse in the Viewport to fly: WASD + QE, mouse-look.");
    ImGui::End();

    drawHierarchyPanel();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport");
    m_viewportFocused = ImGui::IsWindowFocused() || ImGui::IsWindowHovered();

    ImVec2 size = ImGui::GetContentRegionAvail();
    if (size.x > 0 && size.y > 0 &&
        (static_cast<uint32_t>(size.x) != m_framebuffer->getSpec().width ||
         static_cast<uint32_t>(size.y) != m_framebuffer->getSpec().height)) {
        m_framebuffer->resize(static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y));
        m_viewportSize = {size.x, size.y};
        m_editorCamera.setAspectRatio(size.x / size.y);
    }

    ImGui::Image(static_cast<ImTextureID>(static_cast<intptr_t>(m_framebuffer->getColorAttachmentRendererId())),
                 size, ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();
    ImGui::PopStyleVar();
}

}  // namespace molly
