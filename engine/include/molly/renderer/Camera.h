#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace molly {

// A free-fly perspective camera (position + pitch/yaw + FOV). Used directly
// as the editor's debug/fly camera in Milestone 2; the gameplay top-down
// camera (fixed-angle follow) reuses this same class with different control
// logic layered on top (Milestone 7+).
class PerspectiveCamera {
public:
    PerspectiveCamera() = default;
    PerspectiveCamera(float fovDegrees, float aspectRatio, float nearClip, float farClip);

    void setPosition(const glm::vec3& position) { m_position = position; recalculate(); }
    const glm::vec3& getPosition() const { return m_position; }

    // Degrees. Pitch is clamped to [-89, 89] to avoid the view flipping past straight up/down.
    void setPitchYaw(float pitchDegrees, float yawDegrees);
    float getPitch() const { return m_pitch; }
    float getYaw() const { return m_yaw; }

    void setAspectRatio(float aspectRatio) { m_aspectRatio = aspectRatio; recalculate(); }
    void setFov(float fovDegrees) { m_fovDegrees = fovDegrees; recalculate(); }
    float getFov() const { return m_fovDegrees; }
    float getAspectRatio() const { return m_aspectRatio; }

    void setClipPlanes(float nearClip, float farClip) {
        m_nearClip = nearClip;
        m_farClip = farClip;
        recalculate();
    }
    float getNearClip() const { return m_nearClip; }
    float getFarClip() const { return m_farClip; }

    glm::vec3 getForward() const { return m_forward; }
    glm::vec3 getRight() const { return m_right; }
    glm::vec3 getUp() const { return m_up; }

    const glm::mat4& getViewMatrix() const { return m_view; }
    const glm::mat4& getProjectionMatrix() const { return m_projection; }
    glm::mat4 getViewProjectionMatrix() const { return m_projection * m_view; }

private:
    void recalculate();

    glm::vec3 m_position{0.0f, 0.0f, 3.0f};
    float m_pitch = 0.0f;  // degrees
    float m_yaw = -90.0f;  // degrees; -90 faces -Z, matching a Y-up right-handed default forward

    float m_fovDegrees = 60.0f;
    float m_aspectRatio = 16.0f / 9.0f;
    float m_nearClip = 0.1f;
    float m_farClip = 1000.0f;

    glm::vec3 m_forward{0.0f, 0.0f, -1.0f};
    glm::vec3 m_right{1.0f, 0.0f, 0.0f};
    glm::vec3 m_up{0.0f, 1.0f, 0.0f};

    glm::mat4 m_view{1.0f};
    glm::mat4 m_projection{1.0f};
};

}  // namespace molly
