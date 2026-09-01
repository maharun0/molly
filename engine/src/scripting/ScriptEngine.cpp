#include "molly/scripting/ScriptEngine.h"

#include <algorithm>
#include <fstream>
#include <sstream>

#include <GLFW/glfw3.h>

#include "molly/core/Application.h"
#include "molly/core/Log.h"
#include "molly/ecs/Components.h"
#include "molly/ecs/Entity.h"
#include "molly/ecs/Scene.h"
#include "molly/ecs/Systems/CharacterControllerSystem.h"
#include "molly/scene/SceneSerializer.h"
#include "molly/scripting/ScriptComponent.h"

namespace molly {

static std::string readFile(const std::string& path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) return {};
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void ScriptEngine::init(Scene* scene) {
    m_scene = scene;

    // Deliberately not `os`/`io`/`require`/`dofile`/`loadfile` — scripts can
    // only do what the `molly` API surface explicitly allows (PLAN.md §13.5).
    m_lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::string);

    registerBindings();
}

void ScriptEngine::shutdown() {
    m_lua = sol::state();
    m_scene = nullptr;
}

void ScriptEngine::registerBindings() {
    m_lua.new_usertype<glm::vec3>("vec3", sol::constructors<glm::vec3(), glm::vec3(float, float, float)>(),
                                   "x", &glm::vec3::x, "y", &glm::vec3::y, "z", &glm::vec3::z);

    m_lua.new_usertype<TransformComponent>(
        "Transform", "position", &TransformComponent::position, "rotation",
        &TransformComponent::rotation, "scale", &TransformComponent::scale);

    m_lua.new_usertype<HealthComponent>("Health", "maxHealth", &HealthComponent::maxHealth, "health",
                                         &HealthComponent::health);

    m_lua.new_usertype<Entity>(
        "Entity", "tag",
        sol::property([](Entity& e) -> std::string { return e.getComponent<TagComponent>().name; }),
        "transform",
        sol::property([](Entity& e) -> TransformComponent& { return e.getComponent<TransformComponent>(); }),
        "health",
        sol::property([](Entity& e) -> HealthComponent& {
            if (!e.hasComponent<HealthComponent>()) e.addComponent<HealthComponent>();
            return e.getComponent<HealthComponent>();
        }),
        "has_health", [](Entity& e) { return e.hasComponent<HealthComponent>(); }, "is_valid",
        [](Entity& e) { return static_cast<bool>(e); });

    sol::table molly = m_lua.create_named_table("molly");

    molly.set_function("find_entity", [this](const std::string& name) -> sol::object {
        Entity entity = m_scene->findEntityByName(name);
        if (!entity) return sol::lua_nil;
        return sol::make_object(m_lua, entity);
    });

    molly.set_function("destroy_entity", [this](Entity entity) { m_scene->destroyEntity(entity); });

    molly.set_function("spawn_entity", [this](const std::string& prefabPath, glm::vec3 position) -> sol::object {
        Entity entity = SceneSerializer::instantiatePrefab(*m_scene, prefabPath, position);
        if (!entity) return sol::lua_nil;
        return sol::make_object(m_lua, entity);
    });

    molly.set_function("move_towards", [](Entity entity, glm::vec3 target, float speed, float dt) {
        auto& transform = entity.getComponent<TransformComponent>();
        glm::vec3 delta = target - transform.position;
        float dist = glm::length(delta);
        if (dist < 0.0001f) return;
        float step = std::min(dist, speed * dt);
        transform.position += glm::normalize(delta) * step;
    });

    molly.set_function("move_character", [this](Entity entity, float dirX, float dirZ, float speed) {
        moveCharacter(m_scene->getRegistry(), entity, glm::vec2(dirX, dirZ), speed);
    });

    // Spatial query used for melee/ranged targeting (Milestone 10) — finds
    // the nearest entity tagged `tag` within `maxDistance` of `origin`.
    // Grown here rather than added earlier because nothing needed it until
    // combat did (PLAN.md Appendix B's "grows incrementally" note).
    molly.set_function(
        "find_closest_entity", [this](glm::vec3 origin, const std::string& tag, float maxDistance) -> sol::object {
            entt::entity closest = entt::null;
            float closestDistSq = maxDistance * maxDistance;

            auto view = m_scene->getRegistry().view<TagComponent, TransformComponent>();
            for (auto entity : view) {
                auto& t = view.get<TagComponent>(entity);
                if (t.name != tag) continue;

                auto& transform = view.get<TransformComponent>(entity);
                glm::vec3 diff = transform.position - origin;
                float distSq = glm::dot(diff, diff);
                if (distSq <= closestDistSq) {
                    closestDistSq = distSq;
                    closest = entity;
                }
            }

            if (closest == entt::null) return sol::lua_nil;
            return sol::make_object(m_lua, Entity(closest, m_scene));
        });

    molly.set_function("log", [](const std::string& message) { APP_INFO("{0}", message); });

    molly.set_function("play_sound", [](Entity /*entity*/, const std::string& clipPath) {
        // Untracked, non-positional one-shot — matches AudioEngine::playOneShot
        // (PLAN.md §16.3). `entity` is accepted per the planned API shape for a
        // future positional variant but isn't used yet.
        Application::get().getAudioEngine().playOneShot(clipPath);
    });

    sol::table input = molly.create_named("input");
    input.set_function("is_key_down", [](int key) {
        auto* window = Application::get().getWindow().getNativeWindow();
        return glfwGetKey(window, key) == GLFW_PRESS;
    });
    input.set_function("is_mouse_down", [](int button) {
        auto* window = Application::get().getWindow().getNativeWindow();
        return glfwGetMouseButton(window, button) == GLFW_PRESS;
    });

    sol::table key = molly.create_named("Key");
    key["W"] = GLFW_KEY_W;
    key["A"] = GLFW_KEY_A;
    key["S"] = GLFW_KEY_S;
    key["D"] = GLFW_KEY_D;
    key["Space"] = GLFW_KEY_SPACE;
    key["Escape"] = GLFW_KEY_ESCAPE;
    key["Q"] = GLFW_KEY_Q;
    key["E"] = GLFW_KEY_E;

    sol::table mouseButton = molly.create_named("MouseButton");
    mouseButton["Left"] = GLFW_MOUSE_BUTTON_LEFT;
    mouseButton["Right"] = GLFW_MOUSE_BUTTON_RIGHT;
}

sol::table ScriptEngine::loadScriptInstance(const std::string& scriptPath) {
    std::string source = readFile(scriptPath);
    if (source.empty()) {
        MOLLY_ERROR("Failed to read script: {0}", scriptPath);
        return sol::lua_nil;
    }

    auto result = m_lua.safe_script(source, sol::script_pass_on_error);
    if (!result.valid()) {
        sol::error err = result;
        MOLLY_ERROR("Script error in \"{0}\": {1}", scriptPath, err.what());
        return sol::lua_nil;
    }

    if (result.get_type() != sol::type::table) {
        MOLLY_ERROR("Script \"{0}\" did not return a table (expected `return M`)", scriptPath);
        return sol::lua_nil;
    }
    return result;
}

void ScriptEngine::callIfPresent(entt::entity entity, const char* fnName, entt::entity arg) {
    if (!m_scene->getRegistry().all_of<ScriptComponent>(entity)) return;
    auto& script = m_scene->getRegistry().get<ScriptComponent>(entity);
    if (!script.instance.valid()) return;

    sol::protected_function fn = script.instance[fnName];
    if (!fn.valid()) return;

    Entity self(entity, m_scene);
    Entity other(arg, m_scene);
    auto result = fn(self, other);
    if (!result.valid()) {
        sol::error err = result;
        MOLLY_ERROR("Script error in \"{0}\" ({1}): {2}", script.scriptPath, fnName, err.what());
    }
}

void ScriptEngine::callIfPresent(entt::entity entity, const char* fnName, float arg) {
    if (!m_scene->getRegistry().all_of<ScriptComponent>(entity)) return;
    auto& script = m_scene->getRegistry().get<ScriptComponent>(entity);
    if (!script.instance.valid()) return;

    sol::protected_function fn = script.instance[fnName];
    if (!fn.valid()) return;

    Entity self(entity, m_scene);
    auto result = fn(self, arg);
    if (!result.valid()) {
        sol::error err = result;
        MOLLY_ERROR("Script error in \"{0}\" ({1}): {2}", script.scriptPath, fnName, err.what());
    }
}

void ScriptEngine::callIfPresent(entt::entity entity, const char* fnName) {
    if (!m_scene->getRegistry().all_of<ScriptComponent>(entity)) return;
    auto& script = m_scene->getRegistry().get<ScriptComponent>(entity);
    if (!script.instance.valid()) return;

    sol::protected_function fn = script.instance[fnName];
    if (!fn.valid()) return;

    Entity self(entity, m_scene);
    auto result = fn(self);
    if (!result.valid()) {
        sol::error err = result;
        MOLLY_ERROR("Script error in \"{0}\" ({1}): {2}", script.scriptPath, fnName, err.what());
    }
}

void ScriptEngine::ensureStarted(entt::entity entity) {
    auto& script = m_scene->getRegistry().get<ScriptComponent>(entity);
    if (script.started) return;

    if (!script.instance.valid()) {
        script.instance = loadScriptInstance(script.scriptPath);
    }
    script.started = true;
    callIfPresent(entity, "on_start");
}

void ScriptEngine::onUpdate(entt::entity entity, float dt) { callIfPresent(entity, "on_update", dt); }

void ScriptEngine::onTriggerEnter(entt::entity entity, entt::entity other) {
    callIfPresent(entity, "on_trigger_enter", other);
}

void ScriptEngine::onTriggerExit(entt::entity entity, entt::entity other) {
    callIfPresent(entity, "on_trigger_exit", other);
}

void ScriptEngine::onDestroy(entt::entity entity) { callIfPresent(entity, "on_destroy"); }

}  // namespace molly
