#include "molly/ecs/Systems/PhysicsSyncSystem.h"

#include <algorithm>

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

#include "molly/ecs/Components.h"
#include "molly/physics/PhysicsComponents.h"

namespace molly {

static JPH::ObjectLayer objectLayerFor(BodyType type) {
    switch (type) {
        case BodyType::Static:
            return Layers::STATIC;
        default:
            return Layers::DYNAMIC;
    }
}

static JPH::EMotionType motionTypeFor(BodyType type) {
    switch (type) {
        case BodyType::Static:
            return JPH::EMotionType::Static;
        case BodyType::Kinematic:
            return JPH::EMotionType::Kinematic;
        case BodyType::Dynamic:
            return JPH::EMotionType::Dynamic;
    }
    return JPH::EMotionType::Static;
}

static JPH::RefConst<JPH::Shape> makeShape(const ColliderComponent& collider) {
    switch (collider.shape) {
        case ColliderShape::Box:
            return new JPH::BoxShape(JPH::Vec3(collider.halfExtents.x, collider.halfExtents.y,
                                                collider.halfExtents.z));
        case ColliderShape::Sphere:
            return new JPH::SphereShape(collider.radius);
        case ColliderShape::Capsule: {
            float halfHeight = std::max(0.01f, collider.height * 0.5f - collider.radius);
            return new JPH::CapsuleShape(halfHeight, collider.radius);
        }
    }
    return nullptr;
}

// Creates a Jolt body for any entity with Transform+RigidBody+Collider that
// doesn't have one yet. Destruction of bodies whose components/entities are
// removed mid-session is not yet handled — an accepted Milestone 4
// simplification (small, hand-authored test scenes don't exercise it);
// revisit if it becomes a real problem once scenes are more dynamic.
static void syncPhysicsBodies(entt::registry& registry, PhysicsWorld& physicsWorld) {
    auto view = registry.view<TransformComponent, RigidBodyComponent, ColliderComponent>();
    for (auto entity : view) {
        auto& rb = view.get<RigidBodyComponent>(entity);
        if (rb.created) continue;

        const auto& transform = view.get<TransformComponent>(entity);
        const auto& collider = view.get<ColliderComponent>(entity);

        JPH::RefConst<JPH::Shape> shape = makeShape(collider);
        if (!shape) continue;

        JPH::RVec3 position(transform.position.x, transform.position.y, transform.position.z);
        JPH::BodyCreationSettings settings(shape, position, JPH::Quat::sIdentity(),
                                            motionTypeFor(rb.type), objectLayerFor(rb.type));
        settings.mFriction = collider.friction;
        settings.mRestitution = collider.restitution;
        settings.mUserData = static_cast<JPH::uint64>(entt::to_integral(entity));
        if (rb.type == BodyType::Dynamic) {
            settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
            settings.mMassPropertiesOverride.mMass = rb.mass;
        }

        JPH::Body* body = physicsWorld.getBodyInterface().CreateBody(settings);
        if (!body) continue;

        body->SetIsSensor(collider.isTrigger);

        JPH::EActivation activation = rb.type == BodyType::Static ? JPH::EActivation::DontActivate
                                                                    : JPH::EActivation::Activate;
        physicsWorld.getBodyInterface().AddBody(body->GetID(), activation);

        rb.bodyId = body->GetID();
        rb.created = true;
    }
}

static void writeBackTransforms(entt::registry& registry, PhysicsWorld& physicsWorld) {
    auto view = registry.view<TransformComponent, RigidBodyComponent>();
    for (auto entity : view) {
        auto& rb = view.get<RigidBodyComponent>(entity);
        if (!rb.created || rb.type == BodyType::Static) continue;

        auto& transform = view.get<TransformComponent>(entity);
        JPH::RVec3 position = physicsWorld.getBodyInterface().GetPosition(rb.bodyId);
        transform.position = {static_cast<float>(position.GetX()), static_cast<float>(position.GetY()),
                               static_cast<float>(position.GetZ())};
    }
}

void stepPhysics(entt::registry& registry, PhysicsWorld& physicsWorld, float& accumulator,
                  float dt, float fixedDt) {
    syncPhysicsBodies(registry, physicsWorld);

    accumulator += dt;
    constexpr int kMaxStepsPerFrame = 5;  // avoid a spiral of death after a long stall
    int steps = 0;
    while (accumulator >= fixedDt && steps < kMaxStepsPerFrame) {
        physicsWorld.step(fixedDt);
        accumulator -= fixedDt;
        ++steps;
    }

    writeBackTransforms(registry, physicsWorld);
}

}  // namespace molly
