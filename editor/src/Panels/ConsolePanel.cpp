#include "ConsolePanel.h"

#include <imgui.h>
#include <spdlog/fmt/fmt.h>

#include <molly/Molly.h>

namespace molly {

void EditorConsoleSink::sink_it_(const spdlog::details::log_msg& msg) {
    spdlog::memory_buf_t formatted;
    base_sink<std::mutex>::formatter_->format(msg, formatted);

    std::lock_guard<std::mutex> lock(m_messagesMutex);
    m_messages.push_back({fmt::to_string(formatted), static_cast<int>(msg.level)});
    while (m_messages.size() > m_maxMessages) m_messages.pop_front();
}

std::deque<ConsoleMessage> EditorConsoleSink::copyMessages() const {
    std::lock_guard<std::mutex> lock(m_messagesMutex);
    return m_messages;
}

void EditorConsoleSink::clear() {
    std::lock_guard<std::mutex> lock(m_messagesMutex);
    m_messages.clear();
}

ConsolePanel::ConsolePanel() {
    m_sink = std::make_shared<EditorConsoleSink>();
    Log::getEngineLogger()->sinks().push_back(m_sink);
    Log::getAppLogger()->sinks().push_back(m_sink);
}

static ImVec4 colorForLevel(int level) {
    switch (static_cast<spdlog::level::level_enum>(level)) {
        case spdlog::level::trace:
            return {0.6f, 0.6f, 0.6f, 1.0f};
        case spdlog::level::warn:
            return {0.9f, 0.8f, 0.2f, 1.0f};
        case spdlog::level::err:
        case spdlog::level::critical:
            return {0.95f, 0.3f, 0.3f, 1.0f};
        default:
            return {0.85f, 0.85f, 0.85f, 1.0f};
    }
}

void ConsolePanel::onImGuiRender() {
    ImGui::Begin("Console");

    if (ImGui::Button("Clear")) m_sink->clear();
    ImGui::SameLine();
    ImGui::Checkbox("Trace", &m_showTrace);
    ImGui::SameLine();
    ImGui::Checkbox("Info", &m_showInfo);
    ImGui::SameLine();
    ImGui::Checkbox("Warn", &m_showWarn);
    ImGui::SameLine();
    ImGui::Checkbox("Error", &m_showError);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(160.0f);
    ImGui::InputTextWithHint("##search", "Search...", m_searchBuffer, sizeof(m_searchBuffer));

    ImGui::Separator();

    ImGui::BeginChild("ConsoleScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    auto messages = m_sink->copyMessages();
    std::string filter = m_searchBuffer;

    for (const auto& message : messages) {
        auto level = static_cast<spdlog::level::level_enum>(message.level);
        if (level == spdlog::level::trace && !m_showTrace) continue;
        if (level == spdlog::level::info && !m_showInfo) continue;
        if (level == spdlog::level::warn && !m_showWarn) continue;
        if ((level == spdlog::level::err || level == spdlog::level::critical) && !m_showError) continue;
        if (!filter.empty() && message.text.find(filter) == std::string::npos) continue;

        ImGui::PushStyleColor(ImGuiCol_Text, colorForLevel(message.level));
        ImGui::TextUnformatted(message.text.c_str());
        ImGui::PopStyleColor();
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f) ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::End();
}

}  // namespace molly
