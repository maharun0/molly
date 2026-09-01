#include "molly/core/Application.h"

#include <glad/gl.h>

#include "molly/core/Assert.h"
#include "molly/core/Log.h"
#include "molly/core/Time.h"
#include "molly/ui/ImGuiLayer.h"

namespace molly {

Application* Application::s_instance = nullptr;

Application::Application(const std::string& name) {
    MOLLY_ASSERT(!s_instance, "Application already exists");
    s_instance = this;

    WindowProps props;
    props.title = name;
    m_window = std::make_unique<Window>(props);
    m_window->setEventCallback([this](Event& e) { onEvent(e); });
    m_window->setRefreshCallback([this]() {
        renderFrame(0.0f);
        m_window->presentFrame();
    });

    m_imguiLayer = new ImGuiLayer();
    pushOverlay(m_imguiLayer);
}

Application::~Application() { s_instance = nullptr; }

void Application::pushLayer(Layer* layer) { m_layerStack.pushLayer(layer); }

void Application::pushOverlay(Layer* overlay) { m_layerStack.pushOverlay(overlay); }

void Application::close() { m_running = false; }

void Application::onEvent(Event& e) {
    EventDispatcher dispatcher(e);
    dispatcher.dispatch<WindowCloseEvent>([this](WindowCloseEvent& ev) { return onWindowClose(ev); });
    dispatcher.dispatch<WindowResizeEvent>(
        [this](WindowResizeEvent& ev) { return onWindowResize(ev); });

    for (auto it = m_layerStack.rbegin(); it != m_layerStack.rend(); ++it) {
        if (e.handled) break;
        (*it)->onEvent(e);
    }
}

bool Application::onWindowClose(WindowCloseEvent&) {
    m_running = false;
    return true;
}

bool Application::onWindowResize(WindowResizeEvent& e) {
    if (e.getWidth() == 0 || e.getHeight() == 0) {
        m_minimized = true;
        return false;
    }
    m_minimized = false;
    glViewport(0, 0, static_cast<int>(e.getWidth()), static_cast<int>(e.getHeight()));
    return false;
}

void Application::renderFrame(float dt) {
    if (!m_minimized) {
        for (Layer* layer : m_layerStack) layer->onUpdate(dt);
    }

    m_imguiLayer->begin();
    for (Layer* layer : m_layerStack) layer->onImGuiRender();
    m_imguiLayer->end();
}

void Application::run() {
    while (m_running) {
        float time = Time::getSeconds();
        float dt = time - m_lastFrameTime;
        m_lastFrameTime = time;

        if (dt > 0.1f) dt = 0.1f;

        renderFrame(dt);
        m_window->onUpdate();
    }
}

}  // namespace molly
