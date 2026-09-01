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
    void handlePlayerInput(float dt);

    Scene m_scene;
    Entity m_propEntity;
    Entity m_cameraEntity;
    Entity m_playerEntity;
    std::shared_ptr<Material> m_material;
    float m_time = 0.0f;
    int m_spawnCounter = 0;
};

}  // namespace molly
