#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "molly/physics/PhysicsWorld.h"

namespace molly {

// Lazily creates a Jolt CharacterVirtual for entities with
// CharacterControllerComponent+TransformComponent, steps existing
// controllers, and writes the resulting position back to TransformComponent
// (PLAN.md §12.3). No gravity-driven jumping — a small constant downward
// bias keeps the character grounded against floor colliders, matching the
// top-down genre's needs. Unlike rigid bodies, CharacterVirtual is a
// kinematic sweep-and-slide, not an iterative solver, so it's updated once
// per frame with the real (variable) frame dt rather than a fixed-step
// accumulator.
void updateCharacterControllers(entt::registry& registry, PhysicsWorld& physicsWorld, float dt);

// Sets an entity's desired horizontal movement for the next controller
// update. `direction` need not be normalized; a zero vector stops the
// character. Called from gameplay code (hardcoded input in Milestone 4,
// Lua scripts from Milestone 5 onward).
void moveCharacter(entt::registry& registry, entt::entity entity, const glm::vec2& direction,
                    float speed);

}  // namespace molly
