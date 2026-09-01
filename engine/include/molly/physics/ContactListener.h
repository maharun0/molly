#pragma once

#include <functional>

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/ContactListener.h>

namespace molly {

// Forwards trigger enter/exit events for bodies on the TRIGGER layer (or
// colliders flagged as triggers) up to whoever wants them — in Milestone 4
// that's just a log line; Milestone 5 wires this to Lua on_trigger_enter/exit
// script callbacks (PLAN.md §12.4, §13.3).
class MollyContactListener : public JPH::ContactListener {
public:
    using TriggerCallback = std::function<void(JPH::BodyID, JPH::BodyID)>;

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
    TriggerCallback m_onTriggerEnter;
    TriggerCallback m_onTriggerExit;
};

}  // namespace molly
