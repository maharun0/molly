#include "GameLayer.h"

#include <string>

#include <glad/gl.h>
#include <imgui.h>

namespace molly {

GameLayer::GameLayer() : Layer("GameLayer") {}

void GameLayer::onAttach() {
    glEnable(GL_DEPTH_TEST);

    // Enabled here (not in a shipped build generally) mainly to exercise the
    // watcher end-to-end; a real shipped game has no need to watch its own
    // asset files for changes (PLAN.md §14.4).
    Application::get().getResourceManager().setHotReloadEnabled(true);

    buildScene();
}

void GameLayer::buildScene() {
    m_scene = std::make_unique<Scene>();

    auto& resources = Application::get().getResourceManager();
    auto shader = resources.loadShader("assets/shaders/lit.vert", "assets/shaders/lit.frag");

    auto floorMaterial = std::make_shared<Material>();
    floorMaterial->shader = shader;
    floorMaterial->albedoColor = {1.0f, 1.0f, 1.0f};
    floorMaterial->albedoTexture = AssetHandle<Texture2D>("assets/textures/floor_checker.png").get();

    auto wallMaterial = std::make_shared<Material>();
    wallMaterial->shader = shader;
    wallMaterial->albedoColor = {0.45f, 0.42f, 0.4f};

    auto pillarMaterial = std::make_shared<Material>();
    pillarMaterial->shader = shader;
    pillarMaterial->albedoColor = {0.5f, 0.3f, 0.3f};

    auto playerMaterial = std::make_shared<Material>();
    playerMaterial->shader = shader;
    playerMaterial->albedoColor = {0.3f, 0.6f, 0.9f};

    // --- Arena (PLAN.md §31.6): a single bounded arena, perimeter walls +
    // a couple of interior pillars for cover/movement variety. ---
    constexpr float kArenaHalfSize = 10.0f;

    Entity floor = m_scene->createEntity("Floor");
    floor.getComponent<TransformComponent>().position = {0.0f, -0.6f, 0.0f};
    auto& floorRenderer = floor.addComponent<MeshRendererComponent>();
    floorRenderer.mesh = Mesh::createPlane(kArenaHalfSize * 2.0f);
    floorRenderer.material = floorMaterial;
    floor.addComponent<RigidBodyComponent>().type = BodyType::Static;
    auto& floorCollider = floor.addComponent<ColliderComponent>();
    floorCollider.shape = ColliderShape::Box;
    floorCollider.halfExtents = {kArenaHalfSize, 0.1f, kArenaHalfSize};

    auto addBox = [&](const char* name, glm::vec3 position, glm::vec3 halfExtents,
                       std::shared_ptr<Material> material) {
        Entity box = m_scene->createEntity(name);
        auto& t = box.getComponent<TransformComponent>();
        t.position = position;
        t.scale = halfExtents * 2.0f;
        auto& renderer = box.addComponent<MeshRendererComponent>();
        renderer.mesh = Mesh::createCube();
        renderer.material = material;
        box.addComponent<RigidBodyComponent>().type = BodyType::Static;
        auto& collider = box.addComponent<ColliderComponent>();
        collider.shape = ColliderShape::Box;
        collider.halfExtents = halfExtents;
    };

    constexpr float kWallHeight = 1.0f;
    constexpr float kWallThickness = 0.25f;
    addBox("WallNorth", {0.0f, kWallHeight, -kArenaHalfSize}, {kArenaHalfSize, kWallHeight, kWallThickness}, wallMaterial);
    addBox("WallSouth", {0.0f, kWallHeight, kArenaHalfSize}, {kArenaHalfSize, kWallHeight, kWallThickness}, wallMaterial);
    addBox("WallEast", {kArenaHalfSize, kWallHeight, 0.0f}, {kWallThickness, kWallHeight, kArenaHalfSize}, wallMaterial);
    addBox("WallWest", {-kArenaHalfSize, kWallHeight, 0.0f}, {kWallThickness, kWallHeight, kArenaHalfSize}, wallMaterial);

    addBox("Pillar1", {3.0f, 0.6f, 0.0f}, {0.5f, 0.6f, 0.5f}, pillarMaterial);
    addBox("Pillar2", {-3.5f, 0.6f, 2.5f}, {0.5f, 0.6f, 0.5f}, pillarMaterial);
    addBox("Pillar3", {-2.0f, 0.6f, -4.0f}, {0.6f, 0.6f, 0.6f}, pillarMaterial);

    // --- Player ---
    m_playerEntity = m_scene->createEntity("Player");
    m_playerEntity.getComponent<TransformComponent>().scale = {0.6f, 1.8f, 0.6f};
    auto& playerRenderer = m_playerEntity.addComponent<MeshRendererComponent>();
    playerRenderer.mesh = Mesh::createCube();
    playerRenderer.material = playerMaterial;
    auto& playerController = m_playerEntity.addComponent<CharacterControllerComponent>();
    playerController.speed = 3.5f;
    playerController.radius = 0.3f;
    playerController.height = 1.8f;
    auto& playerHealth = m_playerEntity.addComponent<HealthComponent>();
    playerHealth.maxHealth = 100.0f;
    playerHealth.health = 100.0f;
    m_playerEntity.addComponent<ScriptComponent>().scriptPath = "assets/scripts/player.lua";
    m_playerEntity.addComponent<AudioListenerComponent>();

    // --- Wave manager (invisible — drives the core loop, PLAN.md §31.3) ---
    Entity waveManager = m_scene->createEntity("WaveManager");
    waveManager.addComponent<ScriptComponent>().scriptPath = "assets/scripts/wave_manager.lua";

    // --- Background music ---
    Entity music = m_scene->createEntity("BackgroundMusic");
    auto& musicSource = music.addComponent<AudioSourceComponent>();
    musicSource.clipPath = "assets/audio/music_loop.wav";
    musicSource.loop = true;
    musicSource.is3D = false;
    musicSource.volume = 0.5f;

    // --- Directional light ---
    Entity light = m_scene->createEntity("Sun");
    light.getComponent<TransformComponent>().rotation = {-45.0f, 45.0f, 0.0f};
    auto& lightComp = light.addComponent<LightComponent>();
    lightComp.type = LightType::Directional;
    lightComp.color = {1.0f, 0.96f, 0.9f};
    lightComp.intensity = 1.1f;

    // --- Camera: fixed top-down-angled view over the whole arena ---
    Entity camera = m_scene->createEntity("MainCamera");
    auto& cameraComp = camera.addComponent<CameraComponent>();
    auto& window = Application::get().getWindow();
    float aspect = static_cast<float>(window.getWidth()) / static_cast<float>(window.getHeight());
    cameraComp.camera = PerspectiveCamera(55.0f, aspect, 0.1f, 100.0f);
    cameraComp.camera.setPosition({0.0f, 13.0f, 15.0f});
    cameraComp.camera.setPitchYaw(-55.0f, -90.0f);
    cameraComp.primary = true;
}

void GameLayer::onUpdate(float dt) {
    if (m_restartRequested) {
        m_restartRequested = false;
        buildScene();
    }

    glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_scene->onUpdate(dt);
    m_scene->onRender();
}

static sol::optional<sol::table> getGameState(Scene& scene) {
    sol::state& lua = scene.getScriptEngine().getLuaState();
    return lua["GameState"];
}

void GameLayer::drawHud() {
    auto gameState = getGameState(*m_scene);
    if (!gameState) return;
    sol::table gs = *gameState;

    float health = gs.get_or("playerHealth", 100.0f);
    float maxHealth = gs.get_or("playerMaxHealth", 100.0f);
    int wave = gs.get_or("wave", 0);
    bool gameOver = gs.get_or("gameOver", false);
    bool victory = gs.get_or("victory", false);

    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.35f);
    ImGui::Begin("##HUD", nullptr,
                  ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);
    ImGui::Text("Wave %d", wave);
    float healthFrac = maxHealth > 0.0f ? health / maxHealth : 0.0f;
    ImGui::ProgressBar(healthFrac < 0.0f ? 0.0f : healthFrac, ImVec2(200, 0),
                        (std::to_string(static_cast<int>(health > 0 ? health : 0)) + " / " +
                         std::to_string(static_cast<int>(maxHealth)))
                            .c_str());
    ImGui::End();

    if (gameOver || victory) {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::Begin("##EndScreen", nullptr,
                      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                          ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
        if (victory) {
            ImGui::Text("VICTORY — all waves cleared!");
        } else {
            ImGui::Text("GAME OVER — you survived to wave %d", wave);
        }
        if (ImGui::Button("Restart", ImVec2(120, 0))) {
            m_restartRequested = true;
        }
        ImGui::End();
    }
}

void GameLayer::onImGuiRender() {
    drawHud();

    ImGui::Begin("Molly Demo - Milestone 10");
    ImGui::Text("The demo game: wave-survival arena combat.");
    ImGui::Text("Frame time: %.3f ms (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    ImGui::Text("WASD to move. Left click: melee. Right click: ranged.");
    ImGui::Text("Survive the waves — all logic in assets/scripts/*.lua.");
    ImGui::End();
}

}  // namespace molly
