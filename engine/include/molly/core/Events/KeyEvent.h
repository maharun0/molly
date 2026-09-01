#pragma once

#include "molly/core/Events/Event.h"

namespace molly {

class KeyEvent : public Event {
public:
    int getKeyCode() const { return m_keyCode; }

    MOLLY_EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

protected:
    explicit KeyEvent(int keyCode) : m_keyCode(keyCode) {}

    int m_keyCode;
};

class KeyPressedEvent : public KeyEvent {
public:
    KeyPressedEvent(int keyCode, bool isRepeat) : KeyEvent(keyCode), m_isRepeat(isRepeat) {}

    bool isRepeat() const { return m_isRepeat; }

    MOLLY_EVENT_CLASS_TYPE(KeyPressed)

private:
    bool m_isRepeat;
};

class KeyReleasedEvent : public KeyEvent {
public:
    explicit KeyReleasedEvent(int keyCode) : KeyEvent(keyCode) {}

    MOLLY_EVENT_CLASS_TYPE(KeyReleased)
};

}  // namespace molly
