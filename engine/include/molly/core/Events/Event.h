#pragma once

#include <functional>
#include <string>

namespace molly {

enum class EventType {
    None = 0,
    WindowClose,
    WindowResize,
    KeyPressed,
    KeyReleased,
    MouseButtonPressed,
    MouseButtonReleased,
    MouseMoved,
    MouseScrolled,
};

enum EventCategory {
    EventCategoryNone = 0,
    EventCategoryWindow = 1 << 0,
    EventCategoryInput = 1 << 1,
    EventCategoryKeyboard = 1 << 2,
    EventCategoryMouse = 1 << 3,
};

#define MOLLY_EVENT_CLASS_TYPE(type)                                              \
    static EventType getStaticType() { return EventType::type; }                  \
    EventType getEventType() const override { return getStaticType(); }           \
    const char* getName() const override { return #type; }

#define MOLLY_EVENT_CLASS_CATEGORY(category) \
    int getCategoryFlags() const override { return category; }

class Event {
public:
    virtual ~Event() = default;

    virtual EventType getEventType() const = 0;
    virtual const char* getName() const = 0;
    virtual int getCategoryFlags() const = 0;

    bool isInCategory(EventCategory category) const { return getCategoryFlags() & category; }

    bool handled = false;
};

class EventDispatcher {
public:
    explicit EventDispatcher(Event& event) : m_event(event) {}

    template <typename T, typename F>
    bool dispatch(const F& func) {
        if (m_event.getEventType() == T::getStaticType()) {
            m_event.handled |= func(static_cast<T&>(m_event));
            return true;
        }
        return false;
    }

private:
    Event& m_event;
};

}  // namespace molly
