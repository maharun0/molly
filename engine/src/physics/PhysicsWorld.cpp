#include "molly/physics/PhysicsWorld.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <thread>

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/IssueReporting.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/RegisterTypes.h>

#include "molly/core/Log.h"
#include "molly/physics/ContactListener.h"

namespace molly {

// Out-of-line so the implicit std::unique_ptr<MollyContactListener> deleter
// instantiates here, where ContactListener.h's full definition is visible —
// PhysicsWorld.h only forward-declares it (keeps the header light).
PhysicsWorld::PhysicsWorld() = default;
PhysicsWorld::~PhysicsWorld() = default;

static constexpr JPH::uint kMaxBodies = 4096;
static constexpr JPH::uint kNumBodyMutexes = 0;  // 0 = default
static constexpr JPH::uint kMaxBodyPairs = 4096;
static constexpr JPH::uint kMaxContactConstraints = 2048;

static void joltTraceImpl(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    MOLLY_TRACE("[Jolt] {0}", buffer);
}

void PhysicsWorld::init() {
    JPH::RegisterDefaultAllocator();

    JPH::Trace = joltTraceImpl;

    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();

    m_tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);

    unsigned int numThreads = std::max(1u, std::thread::hardware_concurrency() - 1);
    m_jobSystem = std::make_unique<JPH::JobSystemThreadPool>(
        JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, static_cast<int>(numThreads));

    m_system = std::make_unique<JPH::PhysicsSystem>();
    m_system->Init(kMaxBodies, kNumBodyMutexes, kMaxBodyPairs, kMaxContactConstraints,
                   m_broadPhaseLayerInterface, m_objectVsBroadPhaseLayerFilter,
                   m_objectLayerPairFilter);

    m_contactListener = std::make_unique<MollyContactListener>();
    m_contactListener->setTriggerEnterCallback([](JPH::BodyID a, JPH::BodyID b) {
        MOLLY_INFO("Trigger enter: body {0} <-> body {1}", a.GetIndex(), b.GetIndex());
    });
    m_contactListener->setTriggerExitCallback([](JPH::BodyID a, JPH::BodyID b) {
        MOLLY_INFO("Trigger exit: body {0} <-> body {1}", a.GetIndex(), b.GetIndex());
    });
    m_system->SetContactListener(m_contactListener.get());

    MOLLY_INFO("PhysicsWorld initialized ({0} worker threads)", numThreads);
}

void PhysicsWorld::shutdown() {
    m_system.reset();
    m_jobSystem.reset();
    m_tempAllocator.reset();
    m_contactListener.reset();

    JPH::UnregisterTypes();
    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
}

void PhysicsWorld::step(float fixedDt) {
    constexpr int kCollisionSteps = 1;
    m_system->Update(fixedDt, kCollisionSteps, m_tempAllocator.get(), m_jobSystem.get());
}

}  // namespace molly
