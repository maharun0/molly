#include "molly/ecs/Systems/CharacterControllerSystem.h"

#include <algorithm>

#include <Jolt/Jolt.h>

#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyFilter.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/ShapeFilter.h>

#include "molly/ecs/Components.h"
#include "molly/physics/CollisionLayers.h"
#include "molly/physics/PhysicsComponents.h"

namespace molly {

static constexpr float kGroundedBias = -0.15f;  // small constant downward velocity, no real gravity

static void syncCharacterControllers(entt::registry& registry, PhysicsWorld& physicsWorld) {
    auto view = registry.view<TransformComponent, CharacterControllerComponent>();
    for (auto entity : view) {
        auto& controller = view.get<CharacterControllerComponent>(entity);
        if (controller.character) continue;

        const auto& transform = view.get<TransformComponent>(entity);

        float halfHeight = std::max(0.01f, controller.height * 0.5f - controller.radius);
        JPH::RefConst<JPH::Shape> shape = new JPH::CapsuleShape(halfHeight, controller.radius);

        JPH::Ref<JPH::CharacterVirtualSettings> settings = new JPH::CharacterVirtualSettings();
        settings->mShape = shape;
        settings->mMaxSlopeAngle = JPH::DegreesToRadians(45.0f);
        settings->mMass = 70.0f;

        JPH::RVec3 position(transform.position.x, transform.position.y, transform.position.z);
        controller.character = new JPH::CharacterVirtual(settings, position, JPH::Quat::sIdentity(),
                                                           &physicsWorld.getSystem());
    }
}

void updateCharacterControllers(entt::registry& registry, PhysicsWorld& physicsWorld, float dt) {
    syncCharacterControllers(registry, physicsWorld);

    JPH::BroadPhaseLayerFilter broadPhaseFilter;
    JPH::ObjectLayerFilter objectLayerFilter;
    JPH::BodyFilter bodyFilter;
    JPH::ShapeFilter shapeFilter;
    JPH::TempAllocatorImpl allocator(1 * 1024 * 1024);

    auto view = registry.view<TransformComponent, CharacterControllerComponent>();
    for (auto entity : view) {
        auto& controller = view.get<CharacterControllerComponent>(entity);
        if (!controller.character) continue;

        JPH::Vec3 velocity = controller.character->GetLinearVelocity();
        velocity.SetY(kGroundedBias);
        controller.character->SetLinearVelocity(velocity);

        controller.character->Update(dt, JPH::Vec3(0.0f, kGroundedBias, 0.0f), broadPhaseFilter,
                                      objectLayerFilter, bodyFilter, shapeFilter, allocator);

        auto& transform = view.get<TransformComponent>(entity);
        JPH::RVec3 pos = controller.character->GetPosition();
        transform.position = {static_cast<float>(pos.GetX()), static_cast<float>(pos.GetY()),
                               static_cast<float>(pos.GetZ())};
    }
}

void moveCharacter(entt::registry& registry, entt::entity entity, const glm::vec2& direction,
                    float speed) {
    if (!registry.all_of<CharacterControllerComponent>(entity)) return;

    auto& controller = registry.get<CharacterControllerComponent>(entity);
    if (!controller.character) return;  // not created yet; takes effect next frame

    glm::vec2 d = direction;
    float len = glm::length(d);
    if (len > 0.0001f) d /= len;

    JPH::Vec3 velocity = controller.character->GetLinearVelocity();
    velocity.SetX(d.x * speed);
    velocity.SetZ(d.y * speed);
    controller.character->SetLinearVelocity(velocity);
}

}  // namespace molly
