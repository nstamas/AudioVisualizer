#include "ControlPanel.h"
#include "core/Application.h"
#include "core/VisualizationEngine.h"
#include <imgui.h>

namespace AudioVisualizer {
namespace UI {

ControlPanel::ControlPanel(Core::Application* app)
    : app_(app)
    , isVisible_(true)
    , enableMasking_(false)
    , enableOutline_(false)
    , enableShadow_(false)
    , enableSmearing_(false)
    , enableBlur_(false)
    , enableBloom_(false)
    , outlineWidth_(2.0f)
    , smearingAmount_(0.5f)
    , blurRadius_(5.0f)
    , bloomThreshold_(0.8f)
{
    shadowOffset_[0] = 5.0f;
    shadowOffset_[1] = -5.0f;
}

ControlPanel::~ControlPanel() {
}

void ControlPanel::Render() {
    if (!isVisible_) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(350, 700), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(10, 40), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Control Panel", &isVisible_)) {
        if (ImGui::CollapsingHeader("Audio", ImGuiTreeNodeFlags_DefaultOpen)) {
            RenderAudioControls();
        }

        if (ImGui::CollapsingHeader("Visualization", ImGuiTreeNodeFlags_DefaultOpen)) {
            RenderVisualizationControls();
        }

        if (ImGui::CollapsingHeader("Effects")) {
            RenderEffectsControls();
        }

        if (ImGui::CollapsingHeader("Sprites")) {
            RenderSpriteControls();
        }
    }
    ImGui::End();
}

void ControlPanel::RenderAudioControls() {
    // Playback controls
    if (app_->IsPlaying()) {
        if (ImGui::Button("Pause", ImVec2(-1, 0))) {
            app_->Pause();
        }
    } else {
        if (ImGui::Button("Play", ImVec2(-1, 0))) {
            app_->Play();
        }
    }

    if (ImGui::Button("Stop", ImVec2(-1, 0))) {
        app_->Stop();
    }

    ImGui::Separator();

    // Audio source selection
    static int selectedDevice = 0;
    if (ImGui::Combo("Input Device", &selectedDevice, "Default\0Microphone\0Line In\0")) {
        // TODO: Set input device
    }

    // Volume and gain controls
    static float volume = 1.0f;
    ImGui::SliderFloat("Volume", &volume, 0.0f, 2.0f);

    static float gain = 1.0f;
    ImGui::SliderFloat("Gain", &gain, 0.0f, 5.0f);
}

void ControlPanel::RenderVisualizationControls() {
    // Visualization mode selection
    static int currentMode = 0;
    const char* modes[] = { 
        "Bars", "Circular Spectrum", "Waveform", "Particles", "Radial Bars", "Mirrored Bars",
        "Shader: Plasma", "Shader: Tunnel", "Shader: Fractal", "Shader: Kaleidoscope", "Shader: Wave"
    };
    if (ImGui::Combo("Visualization Mode", &currentMode, modes, 11)) {
        app_->GetVisualizationEngine()->SetVisualizationMode(static_cast<Core::VisualizationMode>(currentMode));
    }
    
    // Color scheme selection
    static int currentScheme = 0;
    const char* schemes[] = { "Rainbow", "Fire", "Ocean", "Neon", "Monochrome", "Purple" };
    if (ImGui::Combo("Color Scheme", &currentScheme, schemes, 6)) {
        app_->GetVisualizationEngine()->SetColorScheme(static_cast<Core::ColorScheme>(currentScheme));
    }
    
    ImGui::Separator();
    
    // Beat sensitivity
    static float beatSensitivity = 1.0f;
    if (ImGui::SliderFloat("Beat Sensitivity", &beatSensitivity, 0.0f, 2.0f)) {
        // TODO: Set beat sensitivity
    }

    // Preset duration
    static float presetDuration = 30.0f;
    if (ImGui::SliderFloat("Preset Duration", &presetDuration, 5.0f, 120.0f, "%.0f sec")) {
        // TODO: Set preset duration
    }

    // Smooth transitions
    static bool smoothTransitions = true;
    if (ImGui::Checkbox("Smooth Transitions", &smoothTransitions)) {
        // TODO: Set smooth transitions
    }

    ImGui::Separator();

    // Randomization
    if (ImGui::Button("Random Preset", ImVec2(-1, 0))) {
        // TODO: Load random preset
    }

    if (ImGui::Button("Next Preset", ImVec2(-1, 0))) {
        // TODO: Load next preset
    }

    if (ImGui::Button("Previous Preset", ImVec2(-1, 0))) {
        // TODO: Load previous preset
    }
}

void ControlPanel::RenderEffectsControls() {
    // Masking
    if (ImGui::Checkbox("Masking", &enableMasking_)) {
        // TODO: Enable/disable masking
    }

    if (enableMasking_) {
        ImGui::Indent();
        if (ImGui::Button("Load Mask Image...", ImVec2(-1, 0))) {
            // TODO: Load mask image
        }
        ImGui::Unindent();
    }

    // Outline
    if (ImGui::Checkbox("Outline", &enableOutline_)) {
        // TODO: Enable/disable outline
    }

    if (enableOutline_) {
        ImGui::Indent();
        ImGui::SliderFloat("Width##Outline", &outlineWidth_, 1.0f, 10.0f);
        static float outlineColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        ImGui::ColorEdit4("Color##Outline", outlineColor);
        ImGui::Unindent();
    }

    // Shadow
    if (ImGui::Checkbox("Shadow", &enableShadow_)) {
        // TODO: Enable/disable shadow
    }

    if (enableShadow_) {
        ImGui::Indent();
        ImGui::DragFloat2("Offset##Shadow", shadowOffset_, 1.0f, -50.0f, 50.0f);
        static float shadowBlur = 5.0f;
        ImGui::SliderFloat("Blur##Shadow", &shadowBlur, 0.0f, 20.0f);
        ImGui::Unindent();
    }

    // Smearing
    if (ImGui::Checkbox("Smearing", &enableSmearing_)) {
        // TODO: Enable/disable smearing
    }

    if (enableSmearing_) {
        ImGui::Indent();
        ImGui::SliderFloat("Amount##Smearing", &smearingAmount_, 0.0f, 1.0f);
        ImGui::Unindent();
    }

    // Blur
    if (ImGui::Checkbox("Blur", &enableBlur_)) {
        // TODO: Enable/disable blur
    }

    if (enableBlur_) {
        ImGui::Indent();
        ImGui::SliderFloat("Radius##Blur", &blurRadius_, 0.0f, 20.0f);
        ImGui::Unindent();
    }

    // Bloom
    if (ImGui::Checkbox("Bloom", &enableBloom_)) {
        // TODO: Enable/disable bloom
    }

    if (enableBloom_) {
        ImGui::Indent();
        ImGui::SliderFloat("Threshold##Bloom", &bloomThreshold_, 0.0f, 1.0f);
        static float bloomIntensity = 1.0f;
        ImGui::SliderFloat("Intensity##Bloom", &bloomIntensity, 0.0f, 2.0f);
        ImGui::Unindent();
    }
}

void ControlPanel::RenderSpriteControls() {
    ImGui::Text("Active Sprites: 0");
    
    ImGui::Separator();

    if (ImGui::Button("Add Image...", ImVec2(-1, 0))) {
        // TODO: Add image sprite
    }

    if (ImGui::Button("Add GIF...", ImVec2(-1, 0))) {
        // TODO: Add GIF sprite
    }

    if (ImGui::Button("Add Video...", ImVec2(-1, 0))) {
        // TODO: Add video sprite
    }

    ImGui::Separator();

    if (ImGui::Button("Clear All", ImVec2(-1, 0))) {
        // TODO: Clear all sprites
    }

    // Sprite list
    // TODO: Display list of active sprites with controls
}

} // namespace UI
} // namespace AudioVisualizer
