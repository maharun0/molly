#pragma once

#include <string>

#include "molly/core/Events/Event.h"

namespace molly {

class Layer {
public:
    explicit Layer(const std::string& name = "Layer") : m_debugName(name) {}
    virtual ~Layer() = default;

    virtual void onAttach() {}
    virtual void onDetach() {}
    virtual void onUpdate(float /*dt*/) {}
    virtual void onImGuiRender() {}
    virtual void onEvent(Event& /*event*/) {}

    const std::string& getName() const { return m_debugName; }

protected:
    std::string m_debugName;
};

}  // namespace molly
