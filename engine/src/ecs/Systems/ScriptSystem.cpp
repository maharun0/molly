#include "molly/ecs/Systems/ScriptSystem.h"

#include "molly/scripting/ScriptComponent.h"

namespace molly {

void updateScripts(entt::registry& registry, ScriptEngine& scriptEngine, float dt) {
    auto view = registry.view<ScriptComponent>();
    for (auto entity : view) {
        scriptEngine.ensureStarted(entity);
        scriptEngine.onUpdate(entity, dt);
    }
}

}  // namespace molly
