#pragma once

#include <string>

#include <entt/entt.hpp>

namespace molly {

class Entity;
class PerspectiveCamera;

class Scene {
public:
    Scene() = default;

    Entity createEntity(const std::string& name = "Entity");
    void destroyEntity(Entity entity);

    Entity findEntityByName(const std::string& name);

    // Edit-mode update: no physics/scripts (Milestone 4/5 will add a
    // separate onUpdateRuntime for play mode). Milestone 3 scope only
    // resolves the active camera and issues render draw calls.
    void onUpdate(float dt);
    void onRender();

    entt::registry& getRegistry() { return m_registry; }

private:
    entt::registry m_registry;
    friend class Entity;
};

}  // namespace molly
