#pragma once

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>

#include <glm/glm.hpp>

namespace molly {

enum class BodyType { Static, Kinematic, Dynamic };
enum class ColliderShape { Box, Sphere, Capsule };

struct RigidBodyComponent {
    BodyType type = BodyType::Static;
    float mass = 1.0f;  // dynamic only

    // Runtime-only; not meant to be hand-authored or serialized (PLAN.md §37).
    JPH::BodyID bodyId;
    bool created = false;
};

struct ColliderComponent {
    ColliderShape shape = ColliderShape::Box;
    glm::vec3 halfExtents{0.5f};  // box
    float radius = 0.5f;          // sphere/capsule
    float height = 1.0f;          // capsule
    float friction = 0.6f;
    float restitution = 0.0f;
    bool isTrigger = false;
};

struct CharacterControllerComponent {
    float speed = 3.0f;
    float radius = 0.4f;
    float height = 1.8f;

    // Runtime-only.
    JPH::Ref<JPH::CharacterVirtual> character;
};

}  // namespace molly
