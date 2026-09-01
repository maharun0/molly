#pragma once

#include <memory>
#include <vector>

#include <molly/Molly.h>

namespace molly {

class GameLayer : public Layer {
public:
    GameLayer();

    void onAttach() override;
    void onUpdate(float dt) override;
    void onImGuiRender() override;

private:
    std::shared_ptr<Material> m_material;
    std::vector<std::shared_ptr<Mesh>> m_meshes;
    PerspectiveCamera m_camera;
    Light m_dirLight;
    float m_time = 0.0f;
};

}  // namespace molly
