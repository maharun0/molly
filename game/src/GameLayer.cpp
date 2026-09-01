#include "GameLayer.h"

#include <string>

#include <glad/gl.h>
#include <imgui.h>

namespace molly {

GameLayer::GameLayer() : Layer("GameLayer") {}

void GameLayer::onAttach() {
    glEnable(GL_DEPTH_TEST);

    auto shader = Shader::createFromFiles("assets/shaders/lit.vert", "assets/shaders/lit.frag");

    m_material = std::make_shared<Material>();
    m_material->shader = shader;
    m_material->albedoColor = {0.85f, 0.65f, 0.35f};

    auto floorMaterial = std::make_shared<Material>();
    floorMaterial->shader = shader;
    floorMaterial->albedoColor = {0.35f, 0.38f, 0.42f};

    auto boxMeshes = ModelLoader::load("assets/models/box/Box.gltf");
    std::shared_ptr<Mesh> boxMesh = boxMeshes.empty() ? Mesh::createCube() : boxMeshes[0];
    auto floorMesh = Mesh::createPlane(10.0f);

    // Floor
    Entity floor = m_scene.createEntity("Floor");
    floor.getComponent<TransformComponent>().position = {0.0f, -0.6f, 0.0f};
    auto& floorRenderer = floor.addComponent<MeshRendererComponent>();
    floorRenderer.mesh = floorMesh;
    floorRenderer.material = floorMaterial;

    // Rotating prop
    m_propEntity = m_scene.createEntity("Prop");
    m_propEntity.getComponent<TransformComponent>().position = {0.0f, 0.0f, 0.0f};
    auto& propRenderer = m_propEntity.addComponent<MeshRendererComponent>();
    propRenderer.mesh = boxMesh;
    propRenderer.material = m_material;

    // Directional light
    Entity light = m_scene.createEntity("Sun");
    light.getComponent<TransformComponent>().rotation = {-45.0f, 45.0f, 0.0f};
    auto& lightComp = light.addComponent<LightComponent>();
    lightComp.type = LightType::Directional;
    lightComp.color = {1.0f, 0.96f, 0.9f};
    lightComp.intensity = 1.1f;

    // Camera
    m_cameraEntity = m_scene.createEntity("MainCamera");
    auto& cameraComp = m_cameraEntity.addComponent<CameraComponent>();
    auto& window = Application::get().getWindow();
    float aspect = static_cast<float>(window.getWidth()) / static_cast<float>(window.getHeight());
    cameraComp.camera = PerspectiveCamera(50.0f, aspect, 0.1f, 100.0f);
    cameraComp.camera.setPosition({0.0f, 2.2f, 3.0f});
    cameraComp.camera.setPitchYaw(-45.0f, -90.0f);
    cameraComp.primary = true;
}

void GameLayer::onUpdate(float dt) {
    m_time += dt;

    auto& transform = m_propEntity.getComponent<TransformComponent>();
    transform.rotation.y = m_time * 35.0f;

    glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_scene.onUpdate(dt);
    m_scene.onRender();
}

void GameLayer::onImGuiRender() {
    ImGui::Begin("Molly Demo - Milestone 3");
    ImGui::Text("ECS: entities/components drive the scene (entt).");
    ImGui::Text("Frame time: %.3f ms (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);

    if (ImGui::Button("Spawn Cube")) {
        ++m_spawnCounter;
        Entity cube = m_scene.createEntity("SpawnedCube" + std::to_string(m_spawnCounter));
        auto& t = cube.getComponent<TransformComponent>();
        t.position = {static_cast<float>((m_spawnCounter % 5) - 2) * 1.5f, 0.5f,
                       static_cast<float>((m_spawnCounter / 5) % 5) * 1.5f - 1.5f};
        t.scale = glm::vec3(0.5f);
        auto& renderer = cube.addComponent<MeshRendererComponent>();
        renderer.mesh = Mesh::createCube();
        renderer.material = m_material;
    }

    ImGui::Text("Entities: %d", static_cast<int>(m_scene.getRegistry().view<TagComponent>().size()));
    ImGui::End();
}

}  // namespace molly
