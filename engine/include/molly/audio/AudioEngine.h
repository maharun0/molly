#pragma once

#include <memory>
#include <string>

#include <miniaudio.h>

namespace molly {

// Thin wrapper over miniaudio's high-level ma_engine (PLAN.md §16.1).
// Fire-and-forget one-shot playback goes through here directly;
// persistent/positional sounds (AudioSourceComponent) own their own
// ma_sound, created lazily by AudioSystem, but still play through this
// engine instance.
class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    void init();
    void shutdown();

    // Untracked, non-positional, fire-and-forget playback — the common case
    // for UI clicks, hit impacts, etc. (PLAN.md §16.3).
    void playOneShot(const std::string& path);

    void setGlobalVolume(float volume);

    ma_engine* getEngine() { return &m_engine; }

private:
    ma_engine m_engine{};
    bool m_initialized = false;
};

}  // namespace molly
