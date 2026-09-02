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

    // Presents the frame rendered since the last call, then polls OS events.
    // Order matters: swapping first means the frame we just drew is already
    // on screen before we risk blocking in event polling (see refresh
    // callback note below).
    void onUpdate();

    // Presents the current backbuffer without polling events — used by the
    // window-refresh callback (see below), which must not recursively call
    // glfwPollEvents from inside GLFW's own event-polling call.
    void presentFrame();

    unsigned int getWidth() const { return m_data.width; }
    unsigned int getHeight() const { return m_data.height; }

    void setEventCallback(const EventCallbackFn& callback) { m_data.eventCallback = callback; }

    // Windows enters a blocking modal loop while the user drags/resizes the
    // window by its frame, during which glfwPollEvents() does not return —
    // our normal render loop stalls entirely for the duration of the drag,
    // which is what produced the stale/duplicated-frame glitching. GLFW
    // still invokes this callback during that modal loop, so registering a
    // "redraw now" callback here keeps the window live while it's being
    // moved or resized (PLAN.md's core loop assumption of "one iteration
    // per PollEvents call" doesn't hold during a drag, so this is the
    // escape hatch for that specific case).
    using RefreshCallbackFn = std::function<void()>;
    void setRefreshCallback(const RefreshCallbackFn& callback) { m_data.refreshCallback = callback; }

    void setVSync(bool enabled);
    bool isVSync() const { return m_data.vsync; }

    void setTitle(const std::string& title);
    const std::string& getTitle() const { return m_data.title; }

    // Loads `path` (any stb_image-supported format) and sets it as the
    // window's title-bar/taskbar icon. No-op with a logged error if the
    // file can't be decoded — callers don't need a fallback icon, a running
    // app without a custom icon is a fine degraded state.
    void setIcon(const std::string& path);

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
        RefreshCallbackFn refreshCallback;
    };

    WindowData m_data;
};

}  // namespace molly
