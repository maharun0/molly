// Round-trip serialization tests (PLAN.md §22.1): save a scene, load it back
// into a fresh Scene, and compare. MeshRendererComponent is intentionally
// not covered here — its deserialize path resolves assets through
// ResourceManager via Application::get(), which requires a live GL context/
// window that a plain test executable doesn't have. Every other component
// type is self-contained data and is covered.

#include <cstdio>
#include <filesystem>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <molly/Molly.h>

using namespace molly;
using Catch::Matchers::WithinAbs;

namespace {

// A small RAII wrapper that removes the scratch file when the test ends,
// pass or fail, so repeated runs don't accumulate stale files.
struct ScratchFile {
    std::string path;
    explicit ScratchFile(std::string p) : path(std::move(p)) {
        std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    }
    ~ScratchFile() { std::filesystem::remove(path); }
};

}  // namespace

TEST_CASE("Transform round-trips through scene serialization", "[scene][serialization]") {
    ScratchFile scratch("test_scratch/transform_roundtrip.scene.json");

    Scene scene;
    Entity entity = scene.createEntity("TestEntity");
    auto& transform = entity.getComponent<TransformComponent>();
    transform.position = {1.5f, 2.0f, -3.25f};
    transform.rotation = {0.0f, 90.0f, 0.0f};
    transform.scale = {2.0f, 2.0f, 2.0f};

    REQUIRE(SceneSerializer::serialize(scene, scratch.path));

    Scene loaded;
    REQUIRE(SceneSerializer::deserialize(scratch.path, loaded));

    Entity loadedEntity = loaded.findEntityByName("TestEntity");
    REQUIRE(loadedEntity);

    auto& loadedTransform = loadedEntity.getComponent<TransformComponent>();
    CHECK_THAT(loadedTransform.position.x, WithinAbs(1.5f, 0.0001f));
    CHECK_THAT(loadedTransform.position.y, WithinAbs(2.0f, 0.0001f));
    CHECK_THAT(loadedTransform.position.z, WithinAbs(-3.25f, 0.0001f));
    CHECK_THAT(loadedTransform.rotation.y, WithinAbs(90.0f, 0.0001f));
    CHECK_THAT(loadedTransform.scale.x, WithinAbs(2.0f, 0.0001f));
}

TEST_CASE("Light round-trips through scene serialization", "[scene][serialization]") {
    ScratchFile scratch("test_scratch/light_roundtrip.scene.json");

    Scene scene;
    Entity entity = scene.createEntity("Sun");
    auto& light = entity.addComponent<LightComponent>();
    light.type = LightType::Directional;
    light.color = {1.0f, 0.9f, 0.8f};
    light.intensity = 1.5f;
    light.radius = 12.0f;

    REQUIRE(SceneSerializer::serialize(scene, scratch.path));

    Scene loaded;
    REQUIRE(SceneSerializer::deserialize(scratch.path, loaded));

    Entity loadedEntity = loaded.findEntityByName("Sun");
    REQUIRE(loadedEntity);
    REQUIRE(loadedEntity.hasComponent<LightComponent>());

    auto& loadedLight = loadedEntity.getComponent<LightComponent>();
    CHECK(loadedLight.type == LightType::Directional);
    CHECK_THAT(loadedLight.color.g, WithinAbs(0.9f, 0.0001f));
    CHECK_THAT(loadedLight.intensity, WithinAbs(1.5f, 0.0001f));
}

TEST_CASE("RigidBody and Collider round-trip through scene serialization", "[scene][serialization]") {
    ScratchFile scratch("test_scratch/physics_roundtrip.scene.json");

    Scene scene;
    Entity entity = scene.createEntity("Wall");
    entity.addComponent<RigidBodyComponent>().type = BodyType::Dynamic;
    entity.getComponent<RigidBodyComponent>().mass = 5.0f;

    auto& collider = entity.addComponent<ColliderComponent>();
    collider.shape = ColliderShape::Capsule;
    collider.radius = 0.4f;
    collider.height = 1.8f;
    collider.friction = 0.3f;
    collider.isTrigger = true;

    REQUIRE(SceneSerializer::serialize(scene, scratch.path));

    Scene loaded;
    REQUIRE(SceneSerializer::deserialize(scratch.path, loaded));

    Entity loadedEntity = loaded.findEntityByName("Wall");
    REQUIRE(loadedEntity);

    auto& rb = loadedEntity.getComponent<RigidBodyComponent>();
    CHECK(rb.type == BodyType::Dynamic);
    CHECK_THAT(rb.mass, WithinAbs(5.0f, 0.0001f));

    auto& col = loadedEntity.getComponent<ColliderComponent>();
    CHECK(col.shape == ColliderShape::Capsule);
    CHECK_THAT(col.radius, WithinAbs(0.4f, 0.0001f));
    CHECK(col.isTrigger == true);
}

TEST_CASE("CharacterController round-trips through scene serialization", "[scene][serialization]") {
    ScratchFile scratch("test_scratch/charactercontroller_roundtrip.scene.json");

    Scene scene;
    Entity entity = scene.createEntity("Player");
    auto& cc = entity.addComponent<CharacterControllerComponent>();
    cc.speed = 4.5f;
    cc.radius = 0.35f;
    cc.height = 1.9f;

    REQUIRE(SceneSerializer::serialize(scene, scratch.path));

    Scene loaded;
    REQUIRE(SceneSerializer::deserialize(scratch.path, loaded));

    Entity loadedEntity = loaded.findEntityByName("Player");
    REQUIRE(loadedEntity);

    auto& loadedCC = loadedEntity.getComponent<CharacterControllerComponent>();
    CHECK_THAT(loadedCC.speed, WithinAbs(4.5f, 0.0001f));
}

TEST_CASE("Script round-trips through scene serialization", "[scene][serialization]") {
    ScratchFile scratch("test_scratch/script_roundtrip.scene.json");

    Scene scene;
    Entity entity = scene.createEntity("Player");
    entity.addComponent<ScriptComponent>().scriptPath = "assets/scripts/player.lua";

    REQUIRE(SceneSerializer::serialize(scene, scratch.path));

    Scene loaded;
    REQUIRE(SceneSerializer::deserialize(scratch.path, loaded));

    Entity loadedEntity = loaded.findEntityByName("Player");
    REQUIRE(loadedEntity);

    auto& loadedScript = loadedEntity.getComponent<ScriptComponent>();
    CHECK(loadedScript.scriptPath == "assets/scripts/player.lua");
}

TEST_CASE("Camera round-trips through scene serialization", "[scene][serialization]") {
    ScratchFile scratch("test_scratch/camera_roundtrip.scene.json");

    Scene scene;
    Entity entity = scene.createEntity("MainCamera");
    auto& cam = entity.addComponent<CameraComponent>();
    cam.primary = true;
    cam.camera = PerspectiveCamera(50.0f, 1.777f, 0.2f, 250.0f);
    cam.camera.setPosition({1.0f, 2.0f, 3.0f});
    cam.camera.setPitchYaw(-20.0f, 45.0f);

    REQUIRE(SceneSerializer::serialize(scene, scratch.path));

    Scene loaded;
    REQUIRE(SceneSerializer::deserialize(scratch.path, loaded));

    Entity loadedEntity = loaded.findEntityByName("MainCamera");
    REQUIRE(loadedEntity);

    auto& loadedCam = loadedEntity.getComponent<CameraComponent>();
    CHECK(loadedCam.primary == true);
    CHECK_THAT(loadedCam.camera.getFov(), WithinAbs(50.0f, 0.001f));
    CHECK_THAT(loadedCam.camera.getPosition().z, WithinAbs(3.0f, 0.0001f));
    CHECK_THAT(loadedCam.camera.getPitch(), WithinAbs(-20.0f, 0.0001f));
}

TEST_CASE("Parent/child hierarchy round-trips through scene serialization",
          "[scene][serialization]") {
    ScratchFile scratch("test_scratch/hierarchy_roundtrip.scene.json");

    Scene scene;
    Entity parent = scene.createEntity("Parent");
    Entity child = scene.createEntity("Child");
    parent.getComponent<HierarchyComponent>().children.push_back(child);
    child.getComponent<HierarchyComponent>().parent = parent;

    REQUIRE(SceneSerializer::serialize(scene, scratch.path));

    Scene loaded;
    REQUIRE(SceneSerializer::deserialize(scratch.path, loaded));

    Entity loadedParent = loaded.findEntityByName("Parent");
    Entity loadedChild = loaded.findEntityByName("Child");
    REQUIRE(loadedParent);
    REQUIRE(loadedChild);

    auto& parentHierarchy = loadedParent.getComponent<HierarchyComponent>();
    REQUIRE(parentHierarchy.children.size() == 1);
    CHECK(parentHierarchy.children[0] == static_cast<entt::entity>(loadedChild));

    auto& childHierarchy = loadedChild.getComponent<HierarchyComponent>();
    CHECK(childHierarchy.parent == static_cast<entt::entity>(loadedParent));
}
