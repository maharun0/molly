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
    if (!isOpen) return;
    ImGui::Begin("Console", &isOpen);

    if (ImGui::Button("Clear")) m_sink->clear();

    auto coloredCheckbox = [](const char* label, bool* value, ImVec4 color) {
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::Checkbox(label, value);
        ImGui::PopStyleColor();
    };
    coloredCheckbox("Trace", &m_showTrace, colorForLevel(static_cast<int>(spdlog::level::trace)));
    coloredCheckbox("Info", &m_showInfo, colorForLevel(static_cast<int>(spdlog::level::info)));
    coloredCheckbox("Warn", &m_showWarn, colorForLevel(static_cast<int>(spdlog::level::warn)));
    coloredCheckbox("Error", &m_showError, colorForLevel(static_cast<int>(spdlog::level::err)));
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
        if (ImGui::IsItemClicked()) ImGui::SetClipboardText(message.text.c_str());
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Click to copy");
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f) ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::End();
}

}  // namespace molly
