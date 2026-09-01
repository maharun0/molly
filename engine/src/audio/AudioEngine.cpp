#include "molly/audio/AudioEngine.h"

#include "molly/core/Log.h"

namespace molly {

AudioEngine::AudioEngine() = default;
AudioEngine::~AudioEngine() = default;

void AudioEngine::init() {
    if (m_initialized) return;

    ma_result result = ma_engine_init(nullptr, &m_engine);
    if (result != MA_SUCCESS) {
        MOLLY_ERROR("Failed to initialize audio engine (miniaudio result {0})",
                    static_cast<int>(result));
        return;
    }

    m_initialized = true;
    MOLLY_INFO("AudioEngine initialized");
}

void AudioEngine::shutdown() {
    if (!m_initialized) return;
    ma_engine_uninit(&m_engine);
    m_initialized = false;
}

void AudioEngine::playOneShot(const std::string& path) {
    if (!m_initialized) return;

    ma_result result = ma_engine_play_sound(&m_engine, path.c_str(), nullptr);
    if (result != MA_SUCCESS) {
        MOLLY_WARN("Failed to play sound \"{0}\" (miniaudio result {1})", path,
                   static_cast<int>(result));
    }
}

void AudioEngine::setGlobalVolume(float volume) {
    if (!m_initialized) return;
    ma_engine_set_volume(&m_engine, volume);
}

}  // namespace molly
