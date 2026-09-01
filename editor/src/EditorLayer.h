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
    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<VertexArray> m_vertexArray;
    bool m_showDemoWindow = true;
};

}  // namespace molly
