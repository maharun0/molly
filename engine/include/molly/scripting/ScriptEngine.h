#pragma once

#include <string>

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <sol/sol.hpp>

namespace molly {

class Scene;

// Owns a Lua VM scoped to a single Scene (PLAN.md §13.1) — recreated whenever
// the scene is, so there's no leftover global script state between runs.
// Standard libraries are deliberately restricted: no os/io/require/dofile,
// so scripts can only do what the `molly` API surface explicitly allows
// (a correctness/determinism boundary, not a security one — see §13.5).
class ScriptEngine {
public:
    void init(Scene* scene);
    void shutdown();

    // Lazily loads/instantiates the entity's ScriptComponent (if not
    // already loaded), then runs on_start(entity) exactly once.
    void ensureStarted(entt::entity entity);

    void onUpdate(entt::entity entity, float dt);
    void onTriggerEnter(entt::entity entity, entt::entity other);
    void onTriggerExit(entt::entity entity, entt::entity other);
    void onDestroy(entt::entity entity);

    sol::state& getLuaState() { return m_lua; }

private:
    void registerBindings();
    sol::table loadScriptInstance(const std::string& scriptPath);
    void callIfPresent(entt::entity entity, const char* fnName, entt::entity arg);
    void callIfPresent(entt::entity entity, const char* fnName, float arg);
    void callIfPresent(entt::entity entity, const char* fnName);

    sol::state m_lua;
    Scene* m_scene = nullptr;
};

}  // namespace molly
