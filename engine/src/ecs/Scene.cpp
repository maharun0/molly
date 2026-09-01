#include "molly/ecs/Scene.h"

#include "molly/ecs/Components.h"
#include "molly/ecs/Entity.h"
#include "molly/ecs/Systems/CameraSystem.h"
#include "molly/ecs/Systems/RenderSystem.h"
#include "molly/core/Log.h"

namespace molly {

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

void Scene::onUpdate(float /*dt*/) {
    // Milestone 3 scope: no physics/scripts yet (Milestones 4/5). Nothing to
    // simulate per-frame beyond what onRender reads directly.
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
