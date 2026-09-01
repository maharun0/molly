#pragma once

#include <string>

#include <entt/entt.hpp>

#include "molly/physics/PhysicsWorld.h"

namespace molly {

class Entity;
class PerspectiveCamera;

class Scene {
public:
    Scene();
    ~Scene();

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    Entity createEntity(const std::string& name = "Entity");
    void destroyEntity(Entity entity);

    Entity findEntityByName(const std::string& name);

    // Steps physics (fixed-timestep accumulator) and character controllers.
    // Milestone 4 scope: no scripting yet (Milestone 5) and no separate
    // edit/play mode split yet (Milestone 7) — this is the de facto
    // "runtime" update for now.
    void onUpdate(float dt);
    void onRender();

    entt::registry& getRegistry() { return m_registry; }
    PhysicsWorld& getPhysicsWorld() { return m_physicsWorld; }

private:
    entt::registry m_registry;
    PhysicsWorld m_physicsWorld;
    float m_physicsAccumulator = 0.0f;
    friend class Entity;
};

}  // namespace molly
