#pragma once

#include <memory>

#include <molly/Molly.h>

namespace molly {

// Owns the off-screen Framebuffer the scene is rendered into, the editor's
// free-fly debug camera, and displays the result via ImGui::Image
// (PLAN.md §18.3, §20.2). During Play mode the caller passes the runtime
// scene's own primary camera instead — the editor fly camera is only used
// in edit mode.
class ViewportPanel {
public:
    ViewportPanel();

    void onUpdate(float dt);

    // Renders `scene` and displays it. If `overrideCamera` is non-null (Play
    // mode), that camera is used instead of the editor's own fly camera.
    void onImGuiRender(Scene* scene, PerspectiveCamera* overrideCamera);

    bool isFocused() const { return m_focused; }
    PerspectiveCamera& getEditorCamera() { return m_editorCamera; }

    bool isOpen = true;

private:
    void updateEditorCamera(float dt);

    std::shared_ptr<Framebuffer> m_framebuffer;
    PerspectiveCamera m_editorCamera;
    bool m_focused = false;
    glm::vec2 m_viewportSize{1280.0f, 720.0f};
};

}  // namespace molly
