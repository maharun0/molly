#pragma once

#include <entt/entt.hpp>

#include "molly/renderer/Camera.h"

namespace molly {

// Returns the active primary camera's entity, or entt::null if the scene has none.
entt::entity findPrimaryCamera(entt::registry& registry);

}  // namespace molly
