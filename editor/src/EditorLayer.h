#pragma once

#include <memory>
#include <vector>

#include <molly/Molly.h>

namespace molly {

class EditorLayer : public Layer {
public:
    EditorLayer();

    void onAttach() override;
    void onUpdate(float dt) override;
    void onImGuiRender() override;

private:
    void renderScene();
    void updateEditorCamera(float dt);

    std::shared_ptr<Material> m_material;
    std::vector<std::shared_ptr<Mesh>> m_meshes;
    std::shared_ptr<Framebuffer> m_framebuffer;

    PerspectiveCamera m_camera;
    Light m_dirLight;

    bool m_viewportFocused = false;
    glm::vec2 m_viewportSize{1280.0f, 720.0f};
};

}  // namespace molly
