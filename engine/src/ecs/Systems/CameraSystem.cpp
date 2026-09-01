#include "molly/ecs/Systems/CameraSystem.h"

#include "molly/ecs/Components.h"

namespace molly {

entt::entity findPrimaryCamera(entt::registry& registry) {
    auto view = registry.view<CameraComponent>();
    for (auto entity : view) {
        if (view.get<CameraComponent>(entity).primary) {
            return entity;
        }
    }
    return entt::null;
}

}  // namespace molly
