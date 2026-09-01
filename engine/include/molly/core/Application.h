#pragma once

#include <memory>
#include <string>

#include "molly/core/Events/WindowEvent.h"
#include "molly/core/LayerStack.h"
#include "molly/core/Window.h"

namespace molly {

class ImGuiLayer;

class Application {
public:
    explicit Application(const std::string& name = "Molly Application");
    virtual ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    void run();
    void close();
    void onEvent(Event& e);

    void pushLayer(Layer* layer);
    void pushOverlay(Layer* overlay);

    Window& getWindow() { return *m_window; }
    static Application& get() { return *s_instance; }

private:
    bool onWindowClose(WindowCloseEvent& e);
    bool onWindowResize(WindowResizeEvent& e);

    // Runs one layer-update + ImGui-render pass, without presenting or
    // polling events — shared by the normal run() loop and the window's
    // refresh callback (see Window::setRefreshCallback), which needs to
    // redraw mid-frame while the OS blocks event polling during a window
    // drag/resize.
    void renderFrame(float dt);

    std::unique_ptr<Window> m_window;
    ImGuiLayer* m_imguiLayer = nullptr;
    LayerStack m_layerStack;
    bool m_running = true;
    bool m_minimized = false;
    float m_lastFrameTime = 0.0f;

    static Application* s_instance;
};

}  // namespace molly
