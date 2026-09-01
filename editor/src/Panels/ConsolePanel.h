#pragma once

#include <deque>
#include <mutex>
#include <string>

#include <spdlog/sinks/base_sink.h>

namespace molly {

struct ConsoleMessage {
    std::string text;
    int level;  // spdlog::level::level_enum, kept as int to avoid the header here
};

// A bounded ring-buffer spdlog sink so the Console panel can display recent
// engine/game log lines without re-reading the log file (PLAN.md §8.4,
// §18.3). Thread-safe: spdlog may log from any thread (e.g. the resource
// watcher), the panel only ever reads on the main thread.
class EditorConsoleSink : public spdlog::sinks::base_sink<std::mutex> {
public:
    explicit EditorConsoleSink(size_t maxMessages = 1000) : m_maxMessages(maxMessages) {}

    std::deque<ConsoleMessage> copyMessages() const;
    void clear();

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override;
    void flush_() override {}

private:
    size_t m_maxMessages;
    std::deque<ConsoleMessage> m_messages;
    mutable std::mutex m_messagesMutex;
};

class ConsolePanel {
public:
    ConsolePanel();

    void onImGuiRender();

private:
    std::shared_ptr<EditorConsoleSink> m_sink;
    bool m_showTrace = false;
    bool m_showInfo = true;
    bool m_showWarn = true;
    bool m_showError = true;
    char m_searchBuffer[128] = {0};
};

}  // namespace molly
