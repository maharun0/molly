#pragma once

#include <entt/entt.hpp>

#include "molly/scripting/ScriptEngine.h"

namespace molly {

// Ensures every ScriptComponent's script instance is loaded and its
// on_start(entity) has run, then calls on_update(entity, dt) for each
// (PLAN.md §13.2). A script runtime error is logged and does not halt
// other entities' scripts or the engine (§13.5).
void updateScripts(entt::registry& registry, ScriptEngine& scriptEngine, float dt);

}  // namespace molly
