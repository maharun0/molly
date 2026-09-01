#pragma once

#include <entt/entt.hpp>

#include "molly/core/Assert.h"
#include "molly/ecs/Scene.h"

namespace molly {

class Entity {
public:
    Entity() = default;
    Entity(entt::entity handle, Scene* scene) : m_handle(handle), m_scene(scene) {}

    template <typename T, typename... Args>
    T& addComponent(Args&&... args) {
        MOLLY_ASSERT(!hasComponent<T>(), "Entity already has this component");
        return m_scene->getRegistry().emplace<T>(m_handle, std::forward<Args>(args)...);
    }

    template <typename T>
    T& getComponent() {
        MOLLY_ASSERT(hasComponent<T>(), "Entity does not have this component");
        return m_scene->getRegistry().get<T>(m_handle);
    }

    template <typename T>
    bool hasComponent() const {
        return m_scene->getRegistry().all_of<T>(m_handle);
    }

    template <typename T>
    void removeComponent() {
        m_scene->getRegistry().remove<T>(m_handle);
    }

    operator bool() const { return m_handle != entt::null; }
    operator entt::entity() const { return m_handle; }
    bool operator==(const Entity& other) const {
        return m_handle == other.m_handle && m_scene == other.m_scene;
    }
    bool operator!=(const Entity& other) const { return !(*this == other); }

private:
    entt::entity m_handle{entt::null};
    Scene* m_scene = nullptr;
};

}  // namespace molly
