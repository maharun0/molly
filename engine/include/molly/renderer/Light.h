#pragma once

#include <glm/glm.hpp>

namespace molly {

enum class LightType { Directional, Point };

struct Light {
    LightType type = LightType::Point;
    glm::vec3 position{0.0f};
    glm::vec3 direction{0.0f, -1.0f, 0.0f};
    glm::vec3 color{1.0f};
    float intensity = 1.0f;
    float radius = 10.0f;  // point lights only
};

}  // namespace molly
