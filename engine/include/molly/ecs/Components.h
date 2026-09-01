#pragma once

#include <memory>
#include <string>
#include <vector>

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "molly/renderer/Camera.h"
#include "molly/renderer/Light.h"
#include "molly/renderer/Material.h"
#include "molly/renderer/Mesh.h"

namespace molly {

struct TagComponent {
    std::string name;
};

struct TransformComponent {
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};  // Euler degrees
    glm::vec3 scale{1.0f};

    glm::mat4 getTransform() const {
        glm::mat4 rotationMat = glm::mat4_cast(glm::quat(glm::radians(rotation)));
        return glm::translate(glm::mat4(1.0f), position) * rotationMat *
               glm::scale(glm::mat4(1.0f), scale);
    }
};

struct HierarchyComponent {
    entt::entity parent{entt::null};
    std::vector<entt::entity> children;
};

struct MeshRendererComponent {
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> material;
};

struct CameraComponent {
    PerspectiveCamera camera;
    bool primary = true;
};

struct LightComponent {
    LightType type = LightType::Point;
    glm::vec3 color{1.0f};
    float intensity = 1.0f;
    float radius = 10.0f;
};

// Shared, cross-script-accessible damage state (Milestone 10). Deliberately
// engine data rather than Lua-local script state: combat is fundamentally
// "one entity damages a different entity," which a script's own local
// closure variables can't express — any script can read/write another
// entity's entity.health through the bound Entity usertype.
struct HealthComponent {
    float maxHealth = 100.0f;
    float health = 100.0f;
};

}  // namespace molly
