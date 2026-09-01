#pragma once

#include <memory>

#include <molly/Molly.h>

namespace molly {

class EditorLayer : public Layer {
public:
    EditorLayer();

    void onAttach() override;
    void onUpdate(float dt) override;
    void onImGuiRender() override;

private:
    void updateEditorCamera(float dt);
    void drawHierarchyPanel();
    void renderViewport();

    Scene m_scene;
    Entity m_selected;
    std::shared_ptr<Material> m_material;
    std::shared_ptr<Framebuffer> m_framebuffer;

    PerspectiveCamera m_editorCamera;

    bool m_viewportFocused = false;
    glm::vec2 m_viewportSize{1280.0f, 720.0f};
    int m_spawnCounter = 0;
};

}  // namespace molly
