#pragma once

#include <functional>

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/ContactListener.h>
#include <entt/entt.hpp>

namespace JPH {
class PhysicsSystem;
}

namespace molly {

// Forwards trigger enter/exit events for sensor bodies (colliders flagged
// isTrigger) up to whoever wants them — Scene wires this to Lua
// on_trigger_enter/exit script callbacks (PLAN.md §12.4, §13.3). Entity
// identity is recovered from the Jolt body's user data field, which
// PhysicsSyncSystem stamps with the owning entt::entity on body creation.
class MollyContactListener : public JPH::ContactListener {
public:
    using TriggerCallback = std::function<void(entt::entity, entt::entity)>;

    // Must be called once, after the owning PhysicsSystem is constructed,
    // before any contact callbacks can fire (needed to resolve body user
    // data in OnContactRemoved, which only gives body IDs, not live bodies).
    void setPhysicsSystem(JPH::PhysicsSystem* system) { m_physicsSystem = system; }

    void setTriggerEnterCallback(TriggerCallback cb) { m_onTriggerEnter = std::move(cb); }
    void setTriggerExitCallback(TriggerCallback cb) { m_onTriggerExit = std::move(cb); }

    JPH::ValidateResult OnContactValidate(const JPH::Body& body1, const JPH::Body& body2,
                                           JPH::RVec3Arg baseOffset,
                                           const JPH::CollideShapeResult& collisionResult) override;

    void OnContactAdded(const JPH::Body& body1, const JPH::Body& body2,
                         const JPH::ContactManifold& manifold,
                         JPH::ContactSettings& settings) override;

    void OnContactRemoved(const JPH::SubShapeIDPair& subShapePair) override;

private:
    JPH::PhysicsSystem* m_physicsSystem = nullptr;
    TriggerCallback m_onTriggerEnter;
    TriggerCallback m_onTriggerExit;
};

}  // namespace molly
