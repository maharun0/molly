#include "molly/core/Window.h"

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include "molly/core/Assert.h"
#include "molly/core/Events/KeyEvent.h"
#include "molly/core/Events/MouseEvent.h"
#include "molly/core/Events/WindowEvent.h"
#include "molly/core/Log.h"

namespace molly {

static uint8_t s_glfwWindowCount = 0;

static void glfwErrorCallback(int error, const char* description) {
    MOLLY_ERROR("GLFW Error ({0}): {1}", error, description);
}

Window::Window(const WindowProps& props) { init(props); }

Window::~Window() { shutdown(); }

void Window::init(const WindowProps& props) {
    m_data.title = props.title;
    m_data.width = props.width;
    m_data.height = props.height;

    MOLLY_INFO("Creating window \"{0}\" ({1}x{2})", props.title, props.width, props.height);

    if (s_glfwWindowCount == 0) {
        int success = glfwInit();
        MOLLY_ASSERT_UNUSED(success);
        glfwSetErrorCallback(glfwErrorCallback);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef MOLLY_DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

    m_window = glfwCreateWindow(static_cast<int>(props.width), static_cast<int>(props.height),
                                 m_data.title.c_str(), nullptr, nullptr);
    ++s_glfwWindowCount;

    // Below this, the standard five-panel editor layout genuinely can't
    // display anything usefully (plan/EDITOR_UI_PLAN.md §14.1).
    glfwSetWindowSizeLimits(m_window, 1024, 720, GLFW_DONT_CARE, GLFW_DONT_CARE);

    glfwMakeContextCurrent(m_window);
    int gladVersion = gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress));
    if (gladVersion == 0) {
        MOLLY_CRITICAL("Failed to initialize GLAD / OpenGL context");
    } else {
        MOLLY_INFO("OpenGL loaded: {0}.{1}", GLAD_VERSION_MAJOR(gladVersion),
                   GLAD_VERSION_MINOR(gladVersion));
        MOLLY_INFO("  Vendor:   {0}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
        MOLLY_INFO("  Renderer: {0}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    }

    glfwSetWindowUserPointer(m_window, &m_data);
    setVSync(true);

    glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
        auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
        data.width = static_cast<unsigned int>(width);
        data.height = static_cast<unsigned int>(height);
        WindowResizeEvent event(data.width, data.height);
        if (data.eventCallback) data.eventCallback(event);
    });

    glfwSetWindowRefreshCallback(m_window, [](GLFWwindow* window) {
        auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
        if (data.refreshCallback) data.refreshCallback();
    });

    glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window) {
        auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
        WindowCloseEvent event;
        if (data.eventCallback) data.eventCallback(event);
    });

    glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int, int action, int) {
        auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
        if (!data.eventCallback) return;
        switch (action) {
            case GLFW_PRESS: {
                KeyPressedEvent event(key, false);
                data.eventCallback(event);
                break;
            }
            case GLFW_RELEASE: {
                KeyReleasedEvent event(key);
                data.eventCallback(event);
                break;
            }
            case GLFW_REPEAT: {
                KeyPressedEvent event(key, true);
                data.eventCallback(event);
                break;
            }
            default:
                break;
        }
    });

    glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int) {
        auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
        if (!data.eventCallback) return;
        switch (action) {
            case GLFW_PRESS: {
                MouseButtonPressedEvent event(button);
                data.eventCallback(event);
                break;
            }
            case GLFW_RELEASE: {
                MouseButtonReleasedEvent event(button);
                data.eventCallback(event);
                break;
            }
            default:
                break;
        }
    });

    glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset) {
        auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
        MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
        if (data.eventCallback) data.eventCallback(event);
    });

    glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double x, double y) {
        auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
        MouseMovedEvent event(static_cast<float>(x), static_cast<float>(y));
        if (data.eventCallback) data.eventCallback(event);
    });
}

void Window::shutdown() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        --s_glfwWindowCount;
    }
    if (s_glfwWindowCount == 0) {
        glfwTerminate();
    }
}

void Window::onUpdate() {
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void Window::presentFrame() { glfwSwapBuffers(m_window); }

void Window::setVSync(bool enabled) {
    glfwSwapInterval(enabled ? 1 : 0);
    m_data.vsync = enabled;
}

void Window::setTitle(const std::string& title) {
    m_data.title = title;
    glfwSetWindowTitle(m_window, title.c_str());
}

}  // namespace molly
