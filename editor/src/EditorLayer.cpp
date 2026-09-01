#include "EditorLayer.h"

#include <imgui.h>

namespace molly {

EditorLayer::EditorLayer() : Layer("EditorLayer") {}

void EditorLayer::onAttach() {
    m_editScene = std::make_unique<Scene>();
    buildDefaultScene();
}

void EditorLayer::buildDefaultScene() {
    auto& resources = Application::get().getResourceManager();
    resources.setHotReloadEnabled(true);

    auto shader = resources.loadShader("assets/shaders/lit.vert", "assets/shaders/lit.frag");

    m_material = std::make_shared<Material>();
    m_material->shader = shader;
    m_material->albedoColor = {0.8f, 0.8f, 0.85f};

    auto floorMaterial = std::make_shared<Material>();
    floorMaterial->shader = shader;
    floorMaterial->albedoColor = {1.0f, 1.0f, 1.0f};
    floorMaterial->albedoTexture = AssetHandle<Texture2D>("assets/textures/floor_checker.png").get();

    auto boxMeshes = resources.loadModel("assets/models/box/Box.gltf");
    std::shared_ptr<Mesh> boxMesh = boxMeshes.empty() ? Mesh::createCube() : boxMeshes[0];

    Entity floor = m_editScene->createEntity("Floor");
    floor.getComponent<TransformComponent>().position = {0.0f, -0.6f, 0.0f};
    auto& floorRenderer = floor.addComponent<MeshRendererComponent>();
    floorRenderer.mesh = Mesh::createPlane(10.0f);
    floorRenderer.material = floorMaterial;
    floor.addComponent<RigidBodyComponent>().type = BodyType::Static;
    auto& floorCollider = floor.addComponent<ColliderComponent>();
    floorCollider.shape = ColliderShape::Box;
    floorCollider.halfExtents = {5.0f, 0.1f, 5.0f};

    Entity prop = m_editScene->createEntity("Prop");
    auto& propRenderer = prop.addComponent<MeshRendererComponent>();
    propRenderer.mesh = boxMesh;
    propRenderer.material = m_material;

    Entity light = m_editScene->createEntity("Sun");
    light.getComponent<TransformComponent>().rotation = {-45.0f, 45.0f, 0.0f};
    auto& lightComp = light.addComponent<LightComponent>();
    lightComp.type = LightType::Directional;
    lightComp.color = {1.0f, 0.96f, 0.9f};
    lightComp.intensity = 1.1f;

    Entity camera = m_editScene->createEntity("MainCamera");
    auto& cameraComp = camera.addComponent<CameraComponent>();
    cameraComp.camera = PerspectiveCamera(55.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    cameraComp.camera.setPosition({0.0f, 3.0f, 5.0f});
    cameraComp.camera.setPitchYaw(-25.0f, -90.0f);
    cameraComp.primary = true;
}

Scene* EditorLayer::activeScene() {
    return m_playState == EditorPlayState::Edit ? m_editScene.get() : m_runtimeScene.get();
}

void EditorLayer::onPlay() {
    m_runtimeScene = m_editScene->clone();
    m_playState = EditorPlayState::Playing;
    m_hierarchyPanel.setSelected({});
}

void EditorLayer::onStop() {
    m_runtimeScene.reset();
    m_playState = EditorPlayState::Edit;
    m_hierarchyPanel.setSelected({});
}

void EditorLayer::onPause() {
    m_playState =
        m_playState == EditorPlayState::Playing ? EditorPlayState::Paused : EditorPlayState::Playing;
}

void EditorLayer::onUpdate(float dt) {
    if (m_playState == EditorPlayState::Edit) {
        // No physics/scripts in edit mode (PLAN.md §18.4) — only the fly
        // camera moves; the scene itself is static until Play is pressed.
        m_viewportPanel.onUpdate(dt);
    } else if (m_playState == EditorPlayState::Playing) {
        m_runtimeScene->onUpdate(dt);
    }
    // Paused: no onUpdate for the runtime scene, viewport still redraws the frozen frame.
}

void EditorLayer::drawToolbar() {
    ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoScrollbar);

    bool playing = m_playState != EditorPlayState::Edit;
    if (!playing) {
        if (ImGui::Button("Play")) onPlay();
    } else {
        if (ImGui::Button("Stop")) onStop();
        ImGui::SameLine();
        const char* pauseLabel = m_playState == EditorPlayState::Paused ? "Resume" : "Pause";
        if (ImGui::Button(pauseLabel)) onPause();
    }

    ImGui::SameLine();
    const char* stateLabel = m_playState == EditorPlayState::Edit     ? "Edit"
                              : m_playState == EditorPlayState::Playing ? "Playing"
                                                                        : "Paused";
    ImGui::TextDisabled("(%s)", stateLabel);

    ImGui::End();
}

void EditorLayer::onImGuiRender() {
    // Turns the main window into a dockspace host so panels can be freely
    // docked/rearranged (PLAN.md §18.2). A pre-arranged first-launch layout
    // via ImGui's DockBuilder API is listed there as a nice-to-have; left
    // as a follow-up since getting it right needs visual iteration this
    // session can't do — panels currently start floating and the user
    // docks them once, which ImGui then remembers via imgui.ini.
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    drawToolbar();

    ImGui::Begin("Molly Editor - Milestone 7");
    ImGui::Text("Editor shell: Hierarchy, Inspector, Content Browser, Console, Play/Pause/Stop.");
    ImGui::Text("Frame time: %.3f ms (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    ImGui::Text("Hold Right Mouse in the Viewport to fly: WASD + QE, mouse-look (edit mode only).");
    ImGui::End();

    Scene* scene = activeScene();
    m_hierarchyPanel.setContext(scene);
    m_hierarchyPanel.onImGuiRender();
    m_inspectorPanel.onImGuiRender(scene, m_hierarchyPanel.getSelected());
    m_contentBrowserPanel.onImGuiRender();
    m_consolePanel.onImGuiRender();

    PerspectiveCamera* overrideCamera = nullptr;
    if (m_playState != EditorPlayState::Edit && scene) {
        entt::entity camEntity = findPrimaryCamera(scene->getRegistry());
        if (camEntity != entt::null) {
            overrideCamera = &scene->getRegistry().get<CameraComponent>(camEntity).camera;
        }
    }
    m_viewportPanel.onImGuiRender(scene, overrideCamera);
}

}  // namespace molly
