#pragma once

#include <memory>
#include <string>

#include <miniaudio.h>

namespace molly {

struct MaSoundDeleter {
    void operator()(ma_sound* s) const {
        if (s) {
            ma_sound_uninit(s);
            delete s;
        }
    }
};

struct AudioSourceComponent {
    std::string clipPath;
    float volume = 1.0f;
    bool loop = false;
    bool is3D = true;
    bool autoPlay = true;

    // Runtime-only; lazily created by the audio system (PLAN.md §16.2).
    std::unique_ptr<ma_sound, MaSoundDeleter> sound;
    bool started = false;
};

// Marker — identifies which entity's TransformComponent drives the
// miniaudio listener position each frame. If more than one entity has this
// component, the first one found wins (only one listener is meaningful).
//
// `_reserved` is otherwise-unused: entt applies a special zero-storage
// optimization to truly empty (std::is_empty_v) component types, under
// which registry::emplace<T>()/get<T>() return void instead of T& — that
// breaks Entity::addComponent<T>()'s generic `T&` return type. A dummy
// field is the simplest way to opt this type out of that optimization.
struct AudioListenerComponent {
    bool _reserved = true;
};

}  // namespace molly
