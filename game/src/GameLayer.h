#pragma once

#include <memory>

#include <molly/Molly.h>

namespace molly {

class GameLayer : public Layer {
public:
    GameLayer();

    void onAttach() override;
    void onUpdate(float dt) override;
    void onImGuiRender() override;

private:
    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<VertexArray> m_vertexArray;
};

}  // namespace molly
