#pragma once

#include <memory>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include "molly/physics/CollisionLayers.h"

namespace JPH {
class JobSystemThreadPool;
class TempAllocatorImpl;
class ContactListener;
}  // namespace JPH

namespace molly {

class MollyContactListener;

class PhysicsWorld {
public:
    PhysicsWorld();
    ~PhysicsWorld();

    void init();
    void shutdown();

    // Steps the simulation by a fixed amount. Callers accumulate variable
    // frame deltaTime and call this in fixed increments (PLAN.md §27.1).
    void step(float fixedDt);

    JPH::PhysicsSystem& getSystem() { return *m_system; }
    JPH::BodyInterface& getBodyInterface() { return m_system->GetBodyInterface(); }
    MollyContactListener& getContactListener() { return *m_contactListener; }

private:
    std::unique_ptr<JPH::PhysicsSystem> m_system;
    std::unique_ptr<JPH::JobSystemThreadPool> m_jobSystem;
    std::unique_ptr<JPH::TempAllocatorImpl> m_tempAllocator;
    std::unique_ptr<MollyContactListener> m_contactListener;

    BroadPhaseLayerInterfaceImpl m_broadPhaseLayerInterface;
    ObjectVsBroadPhaseLayerFilterImpl m_objectVsBroadPhaseLayerFilter;
    ObjectLayerPairFilterImpl m_objectLayerPairFilter;
};

}  // namespace molly
