#include "molly/physics/ContactListener.h"

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Body/Body.h>

namespace molly {

JPH::ValidateResult MollyContactListener::OnContactValidate(
    const JPH::Body&, const JPH::Body&, JPH::RVec3Arg, const JPH::CollideShapeResult&) {
    return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
}

void MollyContactListener::OnContactAdded(const JPH::Body& body1, const JPH::Body& body2,
                                           const JPH::ContactManifold&, JPH::ContactSettings&) {
    if (body1.IsSensor() || body2.IsSensor()) {
        if (m_onTriggerEnter) m_onTriggerEnter(body1.GetID(), body2.GetID());
    }
}

void MollyContactListener::OnContactRemoved(const JPH::SubShapeIDPair& subShapePair) {
    // OnContactRemoved only gives body IDs, not live Body references, so we
    // can't re-check IsSensor() here (the body may already be gone). At this
    // project's scale (a handful of trigger volumes) firing the callback for
    // every removed contact pair and letting the consumer decide relevance
    // is simpler than tracking sensor pairs separately.
    if (m_onTriggerExit) {
        m_onTriggerExit(subShapePair.GetBody1ID(), subShapePair.GetBody2ID());
    }
}

}  // namespace molly
