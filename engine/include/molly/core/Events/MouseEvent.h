#pragma once

#include "molly/core/Events/Event.h"

namespace molly {

class MouseMovedEvent : public Event {
public:
    MouseMovedEvent(float x, float y) : m_x(x), m_y(y) {}

    float getX() const { return m_x; }
    float getY() const { return m_y; }

    MOLLY_EVENT_CLASS_TYPE(MouseMoved)
    MOLLY_EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

private:
    float m_x;
    float m_y;
};

class MouseScrolledEvent : public Event {
public:
    MouseScrolledEvent(float xOffset, float yOffset) : m_xOffset(xOffset), m_yOffset(yOffset) {}

    float getXOffset() const { return m_xOffset; }
    float getYOffset() const { return m_yOffset; }

    MOLLY_EVENT_CLASS_TYPE(MouseScrolled)
    MOLLY_EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

private:
    float m_xOffset;
    float m_yOffset;
};

class MouseButtonEvent : public Event {
public:
    int getMouseButton() const { return m_button; }

    MOLLY_EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

protected:
    explicit MouseButtonEvent(int button) : m_button(button) {}

    int m_button;
};

class MouseButtonPressedEvent : public MouseButtonEvent {
public:
    explicit MouseButtonPressedEvent(int button) : MouseButtonEvent(button) {}

    MOLLY_EVENT_CLASS_TYPE(MouseButtonPressed)
};

class MouseButtonReleasedEvent : public MouseButtonEvent {
public:
    explicit MouseButtonReleasedEvent(int button) : MouseButtonEvent(button) {}

    MOLLY_EVENT_CLASS_TYPE(MouseButtonReleased)
};

}  // namespace molly
