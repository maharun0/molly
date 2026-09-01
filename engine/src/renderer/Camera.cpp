#include "molly/renderer/Camera.h"

#include <algorithm>
#include <cmath>

namespace molly {

PerspectiveCamera::PerspectiveCamera(float fovDegrees, float aspectRatio, float nearClip,
                                      float farClip)
    : m_fovDegrees(fovDegrees), m_aspectRatio(aspectRatio), m_nearClip(nearClip), m_farClip(farClip) {
    recalculate();
}

void PerspectiveCamera::setPitchYaw(float pitchDegrees, float yawDegrees) {
    m_pitch = std::clamp(pitchDegrees, -89.0f, 89.0f);
    m_yaw = yawDegrees;
    recalculate();
}

void PerspectiveCamera::recalculate() {
    float pitchRad = glm::radians(m_pitch);
    float yawRad = glm::radians(m_yaw);

    m_forward = glm::normalize(glm::vec3(std::cos(yawRad) * std::cos(pitchRad), std::sin(pitchRad),
                                          std::sin(yawRad) * std::cos(pitchRad)));
    m_right = glm::normalize(glm::cross(m_forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    m_up = glm::normalize(glm::cross(m_right, m_forward));

    m_view = glm::lookAt(m_position, m_position + m_forward, m_up);
    m_projection =
        glm::perspective(glm::radians(m_fovDegrees), m_aspectRatio, m_nearClip, m_farClip);
}

}  // namespace molly
