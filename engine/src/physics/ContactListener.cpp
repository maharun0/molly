#include "molly/physics/ContactListener.h"

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/PhysicsSystem.h>

namespace molly {

static entt::entity toEntity(JPH::uint64 userData) {
    return static_cast<entt::entity>(static_cast<entt::id_type>(userData));
}

JPH::ValidateResult MollyContactListener::OnContactValidate(
    const JPH::Body&, const JPH::Body&, JPH::RVec3Arg, const JPH::CollideShapeResult&) {
    return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
}

void MollyContactListener::OnContactAdded(const JPH::Body& body1, const JPH::Body& body2,
                                           const JPH::ContactManifold&, JPH::ContactSettings&) {
    if (!(body1.IsSensor() || body2.IsSensor())) return;
    if (!m_onTriggerEnter) return;

    entt::entity a = toEntity(body1.GetUserData());
    entt::entity b = toEntity(body2.GetUserData());
    m_onTriggerEnter(a, b);
}

void MollyContactListener::OnContactRemoved(const JPH::SubShapeIDPair& subShapePair) {
    // OnContactRemoved only gives body IDs, not live Body references, so
    // IsSensor() can't be re-checked here directly — resolve user data via
    // the BodyInterface instead (works even after one side's body has since
    // been destroyed, in which case GetUserData returns 0 / entt::null).
    if (!m_onTriggerExit || !m_physicsSystem) return;

    const JPH::BodyInterface& bodies = m_physicsSystem->GetBodyInterfaceNoLock();
    entt::entity a = toEntity(bodies.GetUserData(subShapePair.GetBody1ID()));
    entt::entity b = toEntity(bodies.GetUserData(subShapePair.GetBody2ID()));
    m_onTriggerExit(a, b);
}

}  // namespace molly
