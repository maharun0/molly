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
    // `selected` (if valid) is re-drawn as a wireframe overlay on top, so the
    // Hierarchy selection is visible in the 3D view too — Unity/Godot/Unreal
    // all do this; matching TreeNode highlighting alone isn't enough.
    void onImGuiRender(Scene* scene, PerspectiveCamera* overrideCamera, Entity selected = {});

    bool isFocused() const { return m_focused; }
    PerspectiveCamera& getEditorCamera() { return m_editorCamera; }

    // Moves the editor fly camera so `entity` is centered in view, keeping
    // the camera's current look direction — the "F to focus" behavior
    // standard in Unity/Unreal/Blender for jumping to an off-screen or
    // out-of-frame selection.
    void frameEntity(entt::registry& registry, entt::entity entity);

    bool isOpen = true;

private:
    void updateEditorCamera(float dt);
    void drawWireframeMesh(Mesh& mesh, const glm::mat4& model, const glm::mat4& viewProjection,
                            const glm::vec3& color);
    void drawSelectionOutline(entt::registry& registry, entt::entity entity,
                               const glm::mat4& viewProjection);
    // Small wireframe markers for entities that have no mesh to render —
    // lights and cameras — so they're actually visible/locatable in the
    // viewport instead of being invisible points in space (Unity/Godot/
    // Unreal show icon gizmos for the same reason). Editor-camera view only;
    // hidden during Play since that shows the game's own camera output.
    void drawEntityGizmos(entt::registry& registry, const glm::mat4& viewProjection);

    std::shared_ptr<Framebuffer> m_framebuffer;
    std::shared_ptr<Shader> m_outlineShader;
    std::shared_ptr<Mesh> m_gizmoMesh;
    PerspectiveCamera m_editorCamera;
    bool m_focused = false;
    glm::vec2 m_viewportSize{1280.0f, 720.0f};
};

}  // namespace molly
