#pragma once

#include <entt/entt.hpp>

#include "molly/physics/PhysicsWorld.h"

namespace molly {

// Creates/destroys Jolt bodies to match RigidBodyComponent+ColliderComponent
// state, steps the physics world in fixed increments accumulated from the
// variable frame dt, and writes resulting body transforms back into
// TransformComponent (PLAN.md §12, §27.1).
//
// `accumulator` is caller-owned (typically a Scene member) so multiple
// scenes/worlds can each track their own leftover time independently.
void stepPhysics(entt::registry& registry, PhysicsWorld& physicsWorld, float& accumulator,
                  float dt, float fixedDt = 1.0f / 60.0f);

}  // namespace molly
