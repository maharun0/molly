#include "EditorLayer.h"

#include <cstdio>
#include <filesystem>

#include <imgui.h>

#include "DockLayout.h"

namespace molly {

constexpr float kToolbarHeight = 36.0f;
constexpr float kStatusBarHeight = 24.0f;

EditorLayer::EditorLayer() : Layer("EditorLayer") {}

void EditorLayer::onAttach() {
    m_editScene = std::make_unique<Scene>();
    buildDefaultScene();

    m_hierarchyPanel.onCreatePrefabRequested = [this](Entity entity) {
        m_prefabModalTarget = entity;
        m_openCreatePrefabModal = true;
    };
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
    auto& lightTransform = light.getComponent<TransformComponent>();
    lightTransform.position = {0.0f, 4.0f, 0.0f};
    lightTransform.rotation = {-45.0f, 45.0f, 0.0f};
    auto& lightComp = light.addComponent<LightComponent>();
    lightComp.type = LightType::Directional;
    lightComp.color = {1.0f, 0.96f, 0.9f};
    lightComp.intensity = 1.1f;

    Entity camera = m_editScene->createEntity("MainCamera");
    glm::vec3 cameraStartPos = {0.0f, 4.0f, 6.0f};
    // Keep the entity's TransformComponent in sync with the CameraComponent's
    // own PerspectiveCamera position — the two aren't linked by any system,
    // so setting only one would leave the Hierarchy/Inspector-visible
    // Transform (and this entity's gizmo marker) pointing somewhere the
    // camera doesn't actually render from.
    camera.getComponent<TransformComponent>().position = cameraStartPos;
    auto& cameraComp = camera.addComponent<CameraComponent>();
    cameraComp.camera = PerspectiveCamera(55.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    cameraComp.camera.setPosition(cameraStartPos);
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

void EditorLayer::newScene() {
    if (m_playState != EditorPlayState::Edit) onStop();
    m_editScene = std::make_unique<Scene>();
    m_hierarchyPanel.setSelected({});
    Application::get().getWindow().setTitle("Molly Editor - untitled");
}

void EditorLayer::saveScene() {
    std::filesystem::path path(m_scenePathBuffer);
    std::error_code ec;
    if (path.has_parent_path()) std::filesystem::create_directories(path.parent_path(), ec);
    if (SceneSerializer::serialize(*m_editScene, m_scenePathBuffer)) {
        setStatusMessage(std::string("Scene saved: ") + m_scenePathBuffer);
        Application::get().getWindow().setTitle(
            "Molly Editor - " + path.filename().string());
    }
}

void EditorLayer::openScene() {
    auto loaded = std::make_unique<Scene>();
    if (!SceneSerializer::deserialize(m_scenePathBuffer, *loaded)) {
        setStatusMessage(std::string("Failed to open scene: ") + m_scenePathBuffer);
        return;
    }

    if (m_playState != EditorPlayState::Edit) onStop();
    m_editScene = std::move(loaded);
    m_hierarchyPanel.setSelected({});
    setStatusMessage(std::string("Scene opened: ") + m_scenePathBuffer);
    Application::get().getWindow().setTitle(
        "Molly Editor - " + std::filesystem::path(m_scenePathBuffer).filename().string());
}

void EditorLayer::createPrefabFromSelection(Entity target) {
    if (!target) return;

    std::filesystem::path path(m_prefabPathBuffer);
    std::error_code ec;
    if (path.has_parent_path()) std::filesystem::create_directories(path.parent_path(), ec);
    if (SceneSerializer::serializePrefab(target, m_prefabPathBuffer)) {
        setStatusMessage(std::string("Prefab created: ") + m_prefabPathBuffer);
    }
}

void EditorLayer::setStatusMessage(const std::string& message) {
    m_statusMessage = message;
    m_statusMessageTimer = 2.5f;
}

static void handleKeyboardShortcuts(bool& openRequested, bool& saveRequested, bool& newRequested,
                                     bool& playToggleRequested, bool& deleteRequested,
                                     bool& frameSelectedRequested) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantTextInput) return;  // don't steal keys while typing in a text field

    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) saveRequested = true;
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O, false)) openRequested = true;
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_N, false)) newRequested = true;
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_P, false)) playToggleRequested = true;
    if (ImGui::IsKeyPressed(ImGuiKey_Delete, false)) deleteRequested = true;
    if (!io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_F, false)) frameSelectedRequested = true;
}

void EditorLayer::onUpdate(float dt) {
    bool openRequested = false, saveRequested = false, newRequested = false;
    bool playToggleRequested = false, deleteRequested = false, frameSelectedRequested = false;
    handleKeyboardShortcuts(openRequested, saveRequested, newRequested, playToggleRequested,
                             deleteRequested, frameSelectedRequested);
    if (newRequested) newScene();
    if (saveRequested) saveScene();
    if (openRequested) openScene();
    if (playToggleRequested) {
        if (m_playState == EditorPlayState::Edit) onPlay();
        else onStop();
    }
    if (deleteRequested && m_hierarchyPanel.getSelected()) {
        activeScene()->destroyEntity(m_hierarchyPanel.getSelected());
        m_hierarchyPanel.setSelected({});
    }
    if (frameSelectedRequested && m_playState == EditorPlayState::Edit &&
        m_hierarchyPanel.getSelected()) {
        m_viewportPanel.frameEntity(activeScene()->getRegistry(), m_hierarchyPanel.getSelected());
    }

    if (m_statusMessageTimer > 0.0f) {
        m_statusMessageTimer -= dt;
        if (m_statusMessageTimer <= 0.0f) m_statusMessage = "Ready";
    }

    if (m_playState == EditorPlayState::Edit) {
        // No physics/scripts in edit mode (PLAN.md §18.4) — only the fly
        // camera moves; the scene itself is static until Play is pressed.
        m_viewportPanel.onUpdate(dt);
    } else if (m_playState == EditorPlayState::Playing) {
        m_runtimeScene->onUpdate(dt);
    }
    // Paused: no onUpdate for the runtime scene, viewport still redraws the frozen frame.
}

void EditorLayer::drawMenuBar() {
    if (!ImGui::BeginMainMenuBar()) return;

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New Scene", "Ctrl+N")) newScene();
        if (ImGui::MenuItem("Save Scene", "Ctrl+S")) saveScene();
        if (ImGui::MenuItem("Save Scene As...")) m_openSaveSceneAsModal = true;
        if (ImGui::MenuItem("Open Scene", "Ctrl+O")) openScene();
        ImGui::Separator();
        if (ImGui::MenuItem("Exit")) Application::get().close();
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit")) {
        // Undo/Redo are explicitly deferred (PLAN.md §32 Milestone 7.9) —
        // shown disabled rather than omitted so the menu sets correct
        // expectations that it's planned, not forgotten
        // (plan/EDITOR_UI_PLAN.md §9.1).
        ImGui::BeginDisabled(true);
        ImGui::MenuItem("Undo", "Ctrl+Z");
        ImGui::MenuItem("Redo", "Ctrl+Y");
        ImGui::EndDisabled();
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Window")) {
        if (ImGui::MenuItem("Reset Layout")) m_resetLayoutRequested = true;
        ImGui::Separator();
        ImGui::MenuItem("Hierarchy", nullptr, &m_hierarchyPanel.isOpen);
        ImGui::MenuItem("Viewport", nullptr, &m_viewportPanel.isOpen);
        ImGui::MenuItem("Inspector", nullptr, &m_inspectorPanel.isOpen);
        ImGui::MenuItem("Content Browser", nullptr, &m_contentBrowserPanel.isOpen);
        ImGui::MenuItem("Console", nullptr, &m_consolePanel.isOpen);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Help")) {
        ImGui::MenuItem("About Molly (coming soon)", nullptr, false, false);
        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

void EditorLayer::drawToolbar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    float menuBarHeight = ImGui::GetFrameHeight();

    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarHeight));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, kToolbarHeight));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                              ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                              ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
                              ImGuiWindowFlags_NoCollapse;
    ImGui::Begin("##Toolbar", nullptr, flags);

    // Left zone: transport controls.
    bool playing = m_playState != EditorPlayState::Edit;
    if (!playing) {
        if (ImGui::Button("Play")) onPlay();
    } else {
        if (ImGui::Button("Stop")) onStop();
        ImGui::SameLine();
        const char* pauseLabel = m_playState == EditorPlayState::Paused ? "Resume" : "Pause";
        if (ImGui::Button(pauseLabel)) onPause();
    }

    // Right zone: state badge, right-aligned.
    const char* stateLabel = m_playState == EditorPlayState::Edit      ? "Edit Mode"
                              : m_playState == EditorPlayState::Playing ? "Playing"
                                                                        : "Paused";
    ImVec4 badgeColor = m_playState == EditorPlayState::Edit      ? ImVec4(0.30f, 0.32f, 0.36f, 1.0f)
                        : m_playState == EditorPlayState::Playing ? ImVec4(0.24f, 0.55f, 1.0f, 1.0f)
                                                                    : ImVec4(0.90f, 0.72f, 0.30f, 1.0f);
    float badgeWidth = ImGui::CalcTextSize(stateLabel).x + 24.0f;
    ImGui::SameLine(ImGui::GetWindowWidth() - badgeWidth - 12.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, badgeColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, badgeColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, badgeColor);
    ImGui::Button(stateLabel, ImVec2(badgeWidth, 0));
    ImGui::PopStyleColor(3);

    ImGui::End();
}

void EditorLayer::drawStatusBar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(
        ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - kStatusBarHeight));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, kStatusBarHeight));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                              ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                              ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
                              ImGuiWindowFlags_NoCollapse;
    ImGui::Begin("##StatusBar", nullptr, flags);

    ImGui::TextUnformatted(m_statusMessage.c_str());

    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    ImGui::Text("Scene: %s", m_scenePathBuffer);

    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    ImGui::Text("%d selected", m_hierarchyPanel.getSelected() ? 1 : 0);

    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    const char* stateLabel = m_playState == EditorPlayState::Edit      ? "Edit Mode"
                              : m_playState == EditorPlayState::Playing ? "Playing"
                                                                        : "Paused";
    ImGui::TextUnformatted(stateLabel);

    char fpsText[32];
    std::snprintf(fpsText, sizeof(fpsText), "%.1f FPS", ImGui::GetIO().Framerate);
    float fpsWidth = ImGui::CalcTextSize(fpsText).x;
    ImGui::SameLine(ImGui::GetWindowWidth() - fpsWidth - 12.0f);
    ImGui::TextUnformatted(fpsText);

    ImGui::End();
}

void EditorLayer::drawSaveSceneAsModal() {
    if (m_openSaveSceneAsModal) {
        ImGui::OpenPopup("Save Scene As");
        m_openSaveSceneAsModal = false;
    }

    ImGui::SetNextWindowSize(ImVec2(420, 0), ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal("Save Scene As", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Scene Path", m_scenePathBuffer, sizeof(m_scenePathBuffer));
        ImGui::Spacing();
        if (ImGui::Button("Save", ImVec2(120, 0))) {
            saveScene();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

void EditorLayer::drawCreatePrefabModal() {
    if (m_openCreatePrefabModal) {
        ImGui::OpenPopup("Create Prefab From Selection");
        m_openCreatePrefabModal = false;
    }

    ImGui::SetNextWindowSize(ImVec2(420, 0), ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal("Create Prefab From Selection", nullptr,
                                ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Prefab Path", m_prefabPathBuffer, sizeof(m_prefabPathBuffer));
        ImGui::Spacing();
        if (ImGui::Button("Create", ImVec2(120, 0))) {
            createPrefabFromSelection(m_prefabModalTarget);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

void EditorLayer::onImGuiRender() {
    drawMenuBar();
    drawToolbar();

    // Dockspace fills the region between the Toolbar and the Status Bar —
    // reserving those two strips here (rather than passing the raw main
    // viewport to DockSpaceOverViewport) is what stops docked panels from
    // rendering underneath either strip. On first launch (or after a
    // "Reset Layout"), buildDefaultDockLayout() lays out the standard
    // Hierarchy/Viewport/Inspector/Content-Browser+Console quadrant
    // arrangement instead of leaving every panel to ImGui's default
    // cascade-and-overlap placement. See plan/EDITOR_UI_PLAN.md §7, §8.
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 menuBarSize = ImGui::GetFrameHeight() > 0 ? ImVec2(0, ImGui::GetFrameHeight()) : ImVec2(0, 0);
    ImVec2 dockspacePos =
        ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarSize.y + kToolbarHeight);
    ImVec2 dockspaceSize = ImVec2(
        viewport->Size.x, viewport->Size.y - menuBarSize.y - kToolbarHeight - kStatusBarHeight);

    ImGuiID dockspaceId = ImGui::GetID("MollyMainDockspace");

    if (m_resetLayoutRequested || dockspaceNeedsDefaultLayout(dockspaceId)) {
        buildDefaultDockLayout(dockspaceId, dockspaceSize);
        m_resetLayoutRequested = false;
    }

    ImGui::SetNextWindowPos(dockspacePos);
    ImGui::SetNextWindowSize(dockspaceSize);
    ImGuiWindowFlags dockHostFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                                      ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                      ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("##MollyDockHost", nullptr, dockHostFlags);
    ImGui::PopStyleVar(3);
    ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();

    drawStatusBar();
    drawSaveSceneAsModal();
    drawCreatePrefabModal();

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
    m_viewportPanel.onImGuiRender(scene, overrideCamera, m_hierarchyPanel.getSelected());
}

}  // namespace molly
