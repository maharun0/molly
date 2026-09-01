#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace molly {

// Resolves an entity's world transform by walking its HierarchyComponent
// parent chain. Recomputed on demand rather than cached — see PLAN.md §49
// ("Frequently Revisited Questions") for why that's fine at this scale.
glm::mat4 getWorldTransform(entt::registry& registry, entt::entity entity);

// Iterates TransformComponent + MeshRendererComponent, gathers
// LightComponent entities (first directional light + up to 8 point
// lights), and issues draw calls against the currently bound framebuffer
// using the given camera view-projection matrix and world-space position.
void renderScene(entt::registry& registry, const glm::mat4& viewProjection,
                  const glm::vec3& cameraPosition);

}  // namespace molly
