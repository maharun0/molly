#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>

// A small, fixed layer set (PLAN.md §12.1) rather than Jolt's more elaborate
// broad-phase layer configuration options — not needed at this project's scale.

namespace molly::Layers {

static constexpr JPH::ObjectLayer STATIC = 0;
static constexpr JPH::ObjectLayer DYNAMIC = 1;
static constexpr JPH::ObjectLayer PLAYER = 2;
static constexpr JPH::ObjectLayer ENEMY = 3;
static constexpr JPH::ObjectLayer TRIGGER = 4;
static constexpr JPH::ObjectLayer NUM_LAYERS = 5;

}  // namespace molly::Layers

namespace molly::BroadPhaseLayers {

static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
static constexpr JPH::BroadPhaseLayer MOVING(1);
static constexpr JPH::uint NUM_LAYERS = 2;

}  // namespace molly::BroadPhaseLayers

namespace molly {

// Which object layers collide with which. STATIC only interacts with moving
// things; TRIGGER volumes collide with PLAYER/ENEMY (to detect overlap) but
// not with STATIC/DYNAMIC/other TRIGGERs.
class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
public:
    bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override {
        switch (inObject1) {
            case Layers::STATIC:
                return inObject2 == Layers::DYNAMIC || inObject2 == Layers::PLAYER ||
                       inObject2 == Layers::ENEMY;
            case Layers::DYNAMIC:
                return inObject2 != Layers::TRIGGER;
            case Layers::PLAYER:
            case Layers::ENEMY:
                return true;  // collides with everything, including TRIGGER volumes
            case Layers::TRIGGER:
                return inObject2 == Layers::PLAYER || inObject2 == Layers::ENEMY;
            default:
                return false;
        }
    }
};

class BroadPhaseLayerInterfaceImpl : public JPH::BroadPhaseLayerInterface {
public:
    BroadPhaseLayerInterfaceImpl() {
        m_objectToBroadPhase[Layers::STATIC] = BroadPhaseLayers::NON_MOVING;
        m_objectToBroadPhase[Layers::DYNAMIC] = BroadPhaseLayers::MOVING;
        m_objectToBroadPhase[Layers::PLAYER] = BroadPhaseLayers::MOVING;
        m_objectToBroadPhase[Layers::ENEMY] = BroadPhaseLayers::MOVING;
        m_objectToBroadPhase[Layers::TRIGGER] = BroadPhaseLayers::MOVING;
    }

    JPH::uint GetNumBroadPhaseLayers() const override { return BroadPhaseLayers::NUM_LAYERS; }

    JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override {
        return m_objectToBroadPhase[inLayer];
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override {
        switch (static_cast<JPH::BroadPhaseLayer::Type>(inLayer)) {
            case static_cast<JPH::BroadPhaseLayer::Type>(BroadPhaseLayers::NON_MOVING):
                return "NON_MOVING";
            case static_cast<JPH::BroadPhaseLayer::Type>(BroadPhaseLayers::MOVING):
                return "MOVING";
            default:
                return "INVALID";
        }
    }
#endif

private:
    JPH::BroadPhaseLayer m_objectToBroadPhase[Layers::NUM_LAYERS];
};

class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
public:
    bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override {
        switch (inLayer1) {
            case Layers::STATIC:
                return inLayer2 == BroadPhaseLayers::MOVING;
            default:
                return true;
        }
    }
};

}  // namespace molly
