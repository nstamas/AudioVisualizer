#include "PresetManager.h"
#include "core/Application.h"
#include <imgui.h>
#include <cstring>

namespace AudioVisualizer {
namespace UI {

PresetManager::PresetManager(Core::Application* app)
    : app_(app)
    , isVisible_(false)
    , selectedPreset_(0)
{
    std::memset(searchBuffer_, 0, sizeof(searchBuffer_));
    RefreshPresetList();
}

PresetManager::~PresetManager() {
}

void PresetManager::Render() {
    if (!isVisible_) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(375, 625), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(1610, 40), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Preset Manager", &isVisible_)) {
        // Search box
        ImGui::InputTextWithHint("##Search", "Search presets...", searchBuffer_, 
                                sizeof(searchBuffer_));

        ImGui::Separator();

        // Preset list
        ImGui::BeginChild("PresetList", ImVec2(0, -80), true);
        
        for (size_t i = 0; i < presetNames_.size(); ++i) {
            // Filter by search text
            if (searchBuffer_[0] != '\0') {
                if (presetNames_[i].find(searchBuffer_) == std::string::npos) {
                    continue;
                }
            }

            bool isSelected = (selectedPreset_ == static_cast<int>(i));
            if (ImGui::Selectable(presetNames_[i].c_str(), isSelected, 
                                 ImGuiSelectableFlags_AllowDoubleClick)) {
                selectedPreset_ = i;
                if (ImGui::IsMouseDoubleClicked(0)) {
                    LoadPreset(i);
                }
            }
        }

        ImGui::EndChild();

        // Controls
        if (ImGui::Button("Load", ImVec2(-1, 0))) {
            if (selectedPreset_ >= 0 && selectedPreset_ < static_cast<int>(presetNames_.size())) {
                LoadPreset(selectedPreset_);
            }
        }

        if (ImGui::Button("Random", ImVec2(-1, 0))) {
            // TODO: Load random preset
        }

        if (ImGui::Button("Refresh", ImVec2(-1, 0))) {
            RefreshPresetList();
        }
    }
    ImGui::End();
}

void PresetManager::RefreshPresetList() {
    // TODO: Load preset names from visualization engine
    // This is placeholder data
    presetNames_.clear();
    presetNames_.push_back("Cosmic Waves");
    presetNames_.push_back("Neon Dreams");
    presetNames_.push_back("Fractal Journey");
    presetNames_.push_back("Plasma Storm");
    presetNames_.push_back("Digital Ocean");
    presetNames_.push_back("Psychedelic Swirl");
    presetNames_.push_back("Geometric Patterns");
    presetNames_.push_back("Electric Flow");
    presetNames_.push_back("Abstract Chaos");
    presetNames_.push_back("Liquid Rainbow");
}

void PresetManager::LoadPreset(int index) {
    if (index >= 0 && index < static_cast<int>(presetNames_.size())) {
        // TODO: Load preset through visualization engine
    }
}

} // namespace UI
} // namespace AudioVisualizer
