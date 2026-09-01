#pragma once

#include "molly/core/Layer.h"

namespace molly {

class ImGuiLayer : public Layer {
public:
    ImGuiLayer();
    ~ImGuiLayer() override = default;

    void onAttach() override;
    void onDetach() override;
    void onEvent(Event& event) override;

    void begin();
    void end();

    void setBlockEvents(bool block) { m_blockEvents = block; }

private:
    bool m_blockEvents = true;
};

}  // namespace molly
