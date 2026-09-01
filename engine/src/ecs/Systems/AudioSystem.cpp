#include "molly/ecs/Systems/AudioSystem.h"

#include "molly/audio/AudioComponents.h"
#include "molly/core/Log.h"
#include "molly/ecs/Components.h"

namespace molly {

static void syncAudioSources(entt::registry& registry, AudioEngine& audioEngine) {
    auto view = registry.view<TransformComponent, AudioSourceComponent>();
    for (auto entity : view) {
        auto& src = view.get<AudioSourceComponent>(entity);

        if (!src.sound && !src.clipPath.empty()) {
            auto sound = std::unique_ptr<ma_sound, MaSoundDeleter>(new ma_sound());
            ma_uint32 flags = src.is3D ? 0u : static_cast<ma_uint32>(MA_SOUND_FLAG_NO_SPATIALIZATION);
            ma_result result = ma_sound_init_from_file(audioEngine.getEngine(), src.clipPath.c_str(),
                                                        flags, nullptr, nullptr, sound.get());
            if (result != MA_SUCCESS) {
                MOLLY_ERROR("Failed to load audio clip \"{0}\" (miniaudio result {1})", src.clipPath,
                            static_cast<int>(result));
            } else {
                ma_sound_set_looping(sound.get(), src.loop ? MA_TRUE : MA_FALSE);
                ma_sound_set_volume(sound.get(), src.volume);
                src.sound = std::move(sound);
            }
        }

        if (!src.sound) continue;

        if (src.is3D) {
            const auto& transform = view.get<TransformComponent>(entity);
            ma_sound_set_position(src.sound.get(), transform.position.x, transform.position.y,
                                   transform.position.z);
        }

        if (src.autoPlay && !src.started) {
            ma_sound_start(src.sound.get());
            src.started = true;
        }
    }
}

static void syncListener(entt::registry& registry, AudioEngine& audioEngine) {
    auto view = registry.view<TransformComponent, AudioListenerComponent>();
    for (auto entity : view) {
        const auto& transform = view.get<TransformComponent>(entity);
        ma_engine_listener_set_position(audioEngine.getEngine(), 0, transform.position.x,
                                         transform.position.y, transform.position.z);
        break;  // only one listener is meaningful
    }
}

void updateAudio(entt::registry& registry, AudioEngine& audioEngine) {
    syncAudioSources(registry, audioEngine);
    syncListener(registry, audioEngine);
}

}  // namespace molly
