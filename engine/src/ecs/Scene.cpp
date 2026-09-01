#include "molly/ecs/Scene.h"

#include <unordered_map>

#include "molly/ecs/Components.h"
#include "molly/ecs/Entity.h"
#include "molly/ecs/Systems/CameraSystem.h"
#include "molly/ecs/Systems/CharacterControllerSystem.h"
#include "molly/ecs/Systems/PhysicsSyncSystem.h"
#include "molly/ecs/Systems/RenderSystem.h"
#include "molly/ecs/Systems/ScriptSystem.h"
#include "molly/core/Log.h"
#include "molly/physics/ContactListener.h"
#include "molly/physics/PhysicsComponents.h"
#include "molly/scripting/ScriptComponent.h"

namespace molly {

Scene::Scene() {
    m_physicsWorld.init();
    m_scriptEngine.init(this);

    // Trigger events need both the physics layer (which detects the
    // overlap) and the scripting layer (which reacts to it) — Scene is
    // where both are owned, so the wiring lives here rather than in
    // PhysicsWorld itself (which stays scripting-agnostic).
    auto& contactListener = m_physicsWorld.getContactListener();
    contactListener.setTriggerEnterCallback([this](entt::entity a, entt::entity b) {
        m_scriptEngine.onTriggerEnter(a, b);
        m_scriptEngine.onTriggerEnter(b, a);
    });
    contactListener.setTriggerExitCallback([this](entt::entity a, entt::entity b) {
        m_scriptEngine.onTriggerExit(a, b);
        m_scriptEngine.onTriggerExit(b, a);
    });
}

Scene::~Scene() {
    m_scriptEngine.shutdown();
    m_physicsWorld.shutdown();
}

Entity Scene::createEntity(const std::string& name) {
    Entity entity(m_registry.create(), this);
    entity.addComponent<TagComponent>(name.empty() ? "Entity" : name);
    entity.addComponent<TransformComponent>();
    entity.addComponent<HierarchyComponent>();
    return entity;
}

void Scene::destroyEntity(Entity entity) { m_registry.destroy(entity); }

Entity Scene::findEntityByName(const std::string& name) {
    for (auto entity : m_registry.view<TagComponent>()) {
        if (m_registry.get<TagComponent>(entity).name == name) {
            return Entity(entity, this);
        }
    }
    return {};
}

void Scene::onUpdate(float dt) {
    stepPhysics(m_registry, m_physicsWorld, m_physicsAccumulator, dt);
    updateCharacterControllers(m_registry, m_physicsWorld, dt);
    updateScripts(m_registry, m_scriptEngine, dt);
}

std::unique_ptr<Scene> Scene::clone() const {
    auto clone = std::make_unique<Scene>();

    std::unordered_map<entt::entity, entt::entity> entityMap;
    for (auto entity : m_registry.view<TagComponent>()) {
        entityMap[entity] = clone->m_registry.create();
    }

    for (auto& [oldEntity, newEntity] : entityMap) {
        if (auto* c = m_registry.try_get<TagComponent>(oldEntity)) {
            clone->m_registry.emplace<TagComponent>(newEntity, *c);
        }
        if (auto* c = m_registry.try_get<TransformComponent>(oldEntity)) {
            clone->m_registry.emplace<TransformComponent>(newEntity, *c);
        }
        if (auto* c = m_registry.try_get<HierarchyComponent>(oldEntity)) {
            HierarchyComponent copy;
            copy.parent = (c->parent != entt::null) ? entityMap.at(c->parent) : entt::null;
            for (auto child : c->children) copy.children.push_back(entityMap.at(child));
            clone->m_registry.emplace<HierarchyComponent>(newEntity, copy);
        }
        if (auto* c = m_registry.try_get<MeshRendererComponent>(oldEntity)) {
            clone->m_registry.emplace<MeshRendererComponent>(newEntity, *c);  // shares mesh/material
        }
        if (auto* c = m_registry.try_get<CameraComponent>(oldEntity)) {
            clone->m_registry.emplace<CameraComponent>(newEntity, *c);
        }
        if (auto* c = m_registry.try_get<LightComponent>(oldEntity)) {
            clone->m_registry.emplace<LightComponent>(newEntity, *c);
        }
        if (auto* c = m_registry.try_get<RigidBodyComponent>(oldEntity)) {
            RigidBodyComponent copy;
            copy.type = c->type;
            copy.mass = c->mass;
            // bodyId/created deliberately left default — the clone's own
            // PhysicsWorld gets a fresh Jolt body lazily (PhysicsSyncSystem).
            clone->m_registry.emplace<RigidBodyComponent>(newEntity, copy);
        }
        if (auto* c = m_registry.try_get<ColliderComponent>(oldEntity)) {
            clone->m_registry.emplace<ColliderComponent>(newEntity, *c);
        }
        if (auto* c = m_registry.try_get<CharacterControllerComponent>(oldEntity)) {
            CharacterControllerComponent copy;
            copy.speed = c->speed;
            copy.radius = c->radius;
            copy.height = c->height;
            // `character` left null — aliasing the source scene's Jolt
            // CharacterVirtual across two PhysicsWorlds would be unsafe.
            clone->m_registry.emplace<CharacterControllerComponent>(newEntity, copy);
        }
        if (auto* c = m_registry.try_get<ScriptComponent>(oldEntity)) {
            ScriptComponent copy;
            copy.scriptPath = c->scriptPath;
            // `instance`/`started` left default — the clone's own
            // ScriptEngine (a different sol::state) loads its own instance.
            clone->m_registry.emplace<ScriptComponent>(newEntity, copy);
        }
    }

    return clone;
}

void Scene::onRender() {
    entt::entity cameraEntity = findPrimaryCamera(m_registry);
    if (cameraEntity == entt::null) return;

    auto& cameraComponent = m_registry.get<CameraComponent>(cameraEntity);

    // The camera's eye position lives on PerspectiveCamera itself (it drives the
    // view matrix), not on TransformComponent — the two are kept in sync by
    // whoever positions the camera entity (see PLAN.md §49 on this being a
    // known simplification revisited once a dedicated CameraSystem exists).
    renderScene(m_registry, cameraComponent.camera.getViewProjectionMatrix(),
                cameraComponent.camera.getPosition());
}

}  // namespace molly
