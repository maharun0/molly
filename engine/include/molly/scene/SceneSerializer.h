#pragma once

#include <string>

#include <glm/glm.hpp>

#include "molly/ecs/Entity.h"

namespace molly {

class Scene;

// Saves/loads a Scene as human-readable JSON (PLAN.md §15.2, §37). Every
// component type has a serialize/deserialize function pair registered
// against it (see SceneSerializer.cpp) — adding a new component type means
// writing one registration entry, not touching a central switch statement
// (§8.2's "registry of functions" requirement).
class SceneSerializer {
public:
    static bool serialize(Scene& scene, const std::string& path);
    static bool deserialize(const std::string& path, Scene& scene);

    // Serializes a single entity (and its child subtree) as a standalone
    // prefab file, using the same component machinery as full scenes
    // (§15.3).
    static bool serializePrefab(Entity entity, const std::string& path);

    // Loads a prefab file and instantiates it into `scene` at `position`
    // (overriding the prefab's own saved root position). Returns the root
    // entity, or a null Entity if loading failed.
    static Entity instantiatePrefab(Scene& scene, const std::string& path, const glm::vec3& position);
};

}  // namespace molly
