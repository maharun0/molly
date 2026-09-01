#pragma once

#include <entt/entt.hpp>

#include "molly/audio/AudioEngine.h"

namespace molly {

// Lazily creates a persistent ma_sound for each AudioSourceComponent, keeps
// 3D sources' positions in sync with TransformComponent, starts autoPlay
// sources once, and updates the miniaudio listener position from whichever
// entity has AudioListenerComponent (PLAN.md §16.2).
void updateAudio(entt::registry& registry, AudioEngine& audioEngine);

}  // namespace molly
