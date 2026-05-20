#include "LogWindow.h"
#include "utils/Logger.h"
#include <imgui.h>

namespace AudioVisualizer {
namespace UI {

LogWindow::LogWindow()
    : isVisible_(false)
    , autoScroll_(true)
    , levelFilter_(0)
{
}

LogWindow::~LogWindow() {
}

void LogWindow::Render() {
    if (!isVisible_) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(875, 375), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(457, 570), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Log Window", &isVisible_)) {
        // Filter controls
        ImGui::Text("Filter:");
        ImGui::SameLine();
        const char* filterNames[] = { "All", "Info+", "Warn+", "Error" };
        ImGui::SetNextItemWidth(125);
        ImGui::Combo("##levelfilter", &levelFilter_, filterNames, IM_ARRAYSIZE(filterNames));
        
        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &autoScroll_);
        
        ImGui::SameLine();
        if (ImGui::Button("Clear")) {
            // Note: This just hides old messages by scrolling, doesn't actually clear the buffer
        }
        
        ImGui::Separator();
        
        // Log display area
        ImGui::BeginChild("LogScrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
        
        // Get recent log entries
        auto entries = Utils::Logger::GetRecentEntries();
        
        // Display filtered entries
        for (const auto& entry : entries) {
            // Apply filter
            int entryLevel = static_cast<int>(entry.level);
            if (levelFilter_ == 1 && entryLevel < static_cast<int>(Utils::LogLevel::Info)) continue;
            if (levelFilter_ == 2 && entryLevel < static_cast<int>(Utils::LogLevel::Warning)) continue;
            if (levelFilter_ == 3 && entryLevel < static_cast<int>(Utils::LogLevel::Error)) continue;
            
            // Color code by level
            ImVec4 color;
            switch (entry.level) {
                case Utils::LogLevel::Debug:
                    color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f); // Gray
                    break;
                case Utils::LogLevel::Info:
                    color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f); // Light gray
                    break;
                case Utils::LogLevel::Warning:
                    color = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); // Yellow
                    break;
                case Utils::LogLevel::Error:
                    color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f); // Red
                    break;
            }
            
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextUnformatted(("[" + entry.timestamp + "] " + entry.message).c_str());
            ImGui::PopStyleColor();
        }
        
        // Auto-scroll to bottom
        if (autoScroll_ && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
        
        ImGui::EndChild();
    }
    ImGui::End();
}

} // namespace UI
} // namespace AudioVisualizer
