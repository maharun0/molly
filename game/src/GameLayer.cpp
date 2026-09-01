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

    auto wallMaterial = std::make_shared<Material>();
    wallMaterial->shader = shader;
    wallMaterial->albedoColor = {0.5f, 0.3f, 0.3f};

    auto playerMaterial = std::make_shared<Material>();
    playerMaterial->shader = shader;
    playerMaterial->albedoColor = {0.3f, 0.6f, 0.9f};

    auto triggerMaterial = std::make_shared<Material>();
    triggerMaterial->shader = shader;
    triggerMaterial->albedoColor = {0.9f, 0.9f, 0.2f};

    auto boxMeshes = ModelLoader::load("assets/models/box/Box.gltf");
    std::shared_ptr<Mesh> boxMesh = boxMeshes.empty() ? Mesh::createCube() : boxMeshes[0];

    // Static floor: a large flat box collider under a plane mesh.
    Entity floor = m_scene.createEntity("Floor");
    floor.getComponent<TransformComponent>().position = {0.0f, -0.6f, 0.0f};
    auto& floorRenderer = floor.addComponent<MeshRendererComponent>();
    floorRenderer.mesh = Mesh::createPlane(20.0f);
    floorRenderer.material = floorMaterial;
    floor.addComponent<RigidBodyComponent>().type = BodyType::Static;
    auto& floorCollider = floor.addComponent<ColliderComponent>();
    floorCollider.shape = ColliderShape::Box;
    floorCollider.halfExtents = {10.0f, 0.1f, 10.0f};

    // A couple of static wall obstacles to prove collision/sliding works.
    auto addWall = [&](const char* name, glm::vec3 position, glm::vec3 halfExtents) {
        Entity wall = m_scene.createEntity(name);
        auto& t = wall.getComponent<TransformComponent>();
        t.position = position;
        t.scale = halfExtents * 2.0f;
        auto& renderer = wall.addComponent<MeshRendererComponent>();
        renderer.mesh = Mesh::createCube();
        renderer.material = wallMaterial;
        wall.addComponent<RigidBodyComponent>().type = BodyType::Static;
        auto& collider = wall.addComponent<ColliderComponent>();
        collider.shape = ColliderShape::Box;
        collider.halfExtents = halfExtents;
    };
    addWall("Wall1", {3.0f, 0.5f, 0.0f}, {0.5f, 0.5f, 2.0f});
    addWall("Wall2", {-3.0f, 0.5f, 2.0f}, {1.5f, 0.5f, 0.5f});

    // Trigger volume: logs enter/exit as the player walks through it.
    Entity trigger = m_scene.createEntity("TriggerZone");
    trigger.getComponent<TransformComponent>().position = {0.0f, 0.5f, -3.0f};
    trigger.getComponent<TransformComponent>().scale = {2.0f, 1.0f, 2.0f};
    auto& triggerRenderer = trigger.addComponent<MeshRendererComponent>();
    triggerRenderer.mesh = Mesh::createCube();
    triggerRenderer.material = triggerMaterial;
    trigger.addComponent<RigidBodyComponent>().type = BodyType::Static;
    auto& triggerCollider = trigger.addComponent<ColliderComponent>();
    triggerCollider.shape = ColliderShape::Box;
    triggerCollider.halfExtents = {1.0f, 0.5f, 1.0f};
    triggerCollider.isTrigger = true;
    trigger.addComponent<ScriptComponent>().scriptPath = "assets/scripts/trigger_zone.lua";

    // Rotating prop (unrelated to physics — kept from Milestone 3 as a visual anchor).
    m_propEntity = m_scene.createEntity("Prop");
    m_propEntity.getComponent<TransformComponent>().position = {0.0f, 2.0f, 2.5f};
    auto& propRenderer = m_propEntity.addComponent<MeshRendererComponent>();
    propRenderer.mesh = boxMesh;
    propRenderer.material = m_material;

    // Player: kinematic character controller, capsule visualized as a scaled cube.
    m_playerEntity = m_scene.createEntity("Player");
    m_playerEntity.getComponent<TransformComponent>().position = {0.0f, 0.0f, 3.0f};
    m_playerEntity.getComponent<TransformComponent>().scale = {0.6f, 1.8f, 0.6f};
    auto& playerRenderer = m_playerEntity.addComponent<MeshRendererComponent>();
    playerRenderer.mesh = Mesh::createCube();
    playerRenderer.material = playerMaterial;
    auto& playerController = m_playerEntity.addComponent<CharacterControllerComponent>();
    playerController.speed = 3.5f;
    playerController.radius = 0.3f;
    playerController.height = 1.8f;
    m_playerEntity.addComponent<ScriptComponent>().scriptPath = "assets/scripts/player.lua";

    // Directional light
    Entity light = m_scene.createEntity("Sun");
    light.getComponent<TransformComponent>().rotation = {-45.0f, 45.0f, 0.0f};
    auto& lightComp = light.addComponent<LightComponent>();
    lightComp.type = LightType::Directional;
    lightComp.color = {1.0f, 0.96f, 0.9f};
    lightComp.intensity = 1.1f;

    // Camera: fixed top-down-angled view over the whole test arena.
    m_cameraEntity = m_scene.createEntity("MainCamera");
    auto& cameraComp = m_cameraEntity.addComponent<CameraComponent>();
    auto& window = Application::get().getWindow();
    float aspect = static_cast<float>(window.getWidth()) / static_cast<float>(window.getHeight());
    cameraComp.camera = PerspectiveCamera(55.0f, aspect, 0.1f, 100.0f);
    cameraComp.camera.setPosition({0.0f, 7.0f, 9.0f});
    cameraComp.camera.setPitchYaw(-55.0f, -90.0f);
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
    ImGui::Begin("Molly Demo - Milestone 5");
    ImGui::Text("Scripting: player movement + trigger callbacks are Lua, not C++.");
    ImGui::Text("Frame time: %.3f ms (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    ImGui::Text("WASD (handled in assets/scripts/player.lua) to move the blue capsule.");
    ImGui::Text("Walk through the yellow trigger zone and watch the log for enter/exit");
    ImGui::Text("messages (assets/scripts/trigger_zone.lua).");

    if (ImGui::Button("Spawn Cube")) {
        ++m_spawnCounter;
        Entity cube = m_scene.createEntity("SpawnedCube" + std::to_string(m_spawnCounter));
        auto& t = cube.getComponent<TransformComponent>();
        t.position = {static_cast<float>((m_spawnCounter % 5) - 2) * 1.5f, 3.0f,
                       static_cast<float>((m_spawnCounter / 5) % 5) * 1.5f - 1.5f};
        t.scale = glm::vec3(0.5f);
        auto& renderer = cube.addComponent<MeshRendererComponent>();
        renderer.mesh = Mesh::createCube();
        renderer.material = m_material;
        cube.addComponent<RigidBodyComponent>().type = BodyType::Dynamic;
        auto& collider = cube.addComponent<ColliderComponent>();
        collider.shape = ColliderShape::Box;
        collider.halfExtents = glm::vec3(0.25f);
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(dynamic boxes fall onto the floor under gravity)");

    ImGui::Text("Entities: %d", static_cast<int>(m_scene.getRegistry().view<TagComponent>().size()));
    ImGui::End();
}

}  // namespace molly
