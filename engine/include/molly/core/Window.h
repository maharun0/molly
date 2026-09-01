#pragma once

#include <functional>
#include <string>

#include "molly/core/Events/Event.h"

struct GLFWwindow;

namespace molly {

struct WindowProps {
    std::string title = "Molly";
    unsigned int width = 1600;
    unsigned int height = 900;
};

class Window {
public:
    using EventCallbackFn = std::function<void(Event&)>;

    explicit Window(const WindowProps& props = WindowProps());
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    void onUpdate();

    unsigned int getWidth() const { return m_data.width; }
    unsigned int getHeight() const { return m_data.height; }

    void setEventCallback(const EventCallbackFn& callback) { m_data.eventCallback = callback; }
    void setVSync(bool enabled);
    bool isVSync() const { return m_data.vsync; }

    GLFWwindow* getNativeWindow() const { return m_window; }

private:
    void init(const WindowProps& props);
    void shutdown();

    GLFWwindow* m_window = nullptr;

    struct WindowData {
        std::string title;
        unsigned int width = 0;
        unsigned int height = 0;
        bool vsync = true;
        EventCallbackFn eventCallback;
    };

    WindowData m_data;
};

}  // namespace molly
