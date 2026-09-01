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
    void buildScene();
    void drawHud();

    std::unique_ptr<Scene> m_scene;
    Entity m_playerEntity;

    // The Restart button can't destroy m_scene from inside drawHud(): that
    // function is still holding a sol::table reference into the old
    // scene's Lua state (read from GameState) at the point the button is
    // clicked, and that local's destructor runs after the click handler
    // returns — against an already-destroyed lua_State once restart()
    // resets m_scene. Deferring the actual rebuild to the start of the
    // next onUpdate() (before anything touches the scene again) avoids the
    // use-after-free; this crashed the very first time it was exercised.
    bool m_restartRequested = false;
};

}  // namespace molly
