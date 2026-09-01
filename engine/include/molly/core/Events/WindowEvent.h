#pragma once

#include "molly/core/Events/Event.h"

namespace molly {

class WindowCloseEvent : public Event {
public:
    WindowCloseEvent() = default;

    MOLLY_EVENT_CLASS_TYPE(WindowClose)
    MOLLY_EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

class WindowResizeEvent : public Event {
public:
    WindowResizeEvent(unsigned int width, unsigned int height) : m_width(width), m_height(height) {}

    unsigned int getWidth() const { return m_width; }
    unsigned int getHeight() const { return m_height; }

    MOLLY_EVENT_CLASS_TYPE(WindowResize)
    MOLLY_EVENT_CLASS_CATEGORY(EventCategoryWindow)

private:
    unsigned int m_width;
    unsigned int m_height;
};

}  // namespace molly
