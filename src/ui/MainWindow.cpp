#include "MainWindow.h"
#include "core/Application.h"
#include "utils/FileIO.h"
#include "utils/Logger.h"
#include <imgui.h>

namespace AudioVisualizer {
namespace UI {

MainWindow::MainWindow(Core::Application* app)
    : app_(app)
    , showAbout_(false)
    , showSettings_(false)
{
}

MainWindow::~MainWindow() {
}

void MainWindow::Render() {
    // Main menu bar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open Audio File...", "Ctrl+O")) {
                std::string filePath = Utils::FileIO::OpenFileDialog(
                    "Open Audio File", 
                    "Audio Files (*.wav;*.mp3;*.ogg;*.flac)\0*.wav;*.mp3;*.ogg;*.flac\0All Files (*.*)\0*.*\0"
                );
                if (!filePath.empty()) {
                    if (app_->LoadAudioFile(filePath)) {
                        Utils::Logger::Info("Successfully loaded audio file");
                        app_->Play();  // Auto-play after loading
                    } else {
                        Utils::Logger::Error("Failed to load audio file");
                    }
                }
            }
            if (ImGui::MenuItem("Save Configuration", "Ctrl+S")) {
                // TODO: Save configuration
            }
            if (ImGui::MenuItem("Load Configuration", "Ctrl+L")) {
                // TODO: Load configuration
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                // TODO: Request application exit
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Recording")) {
            if (app_->IsRecording()) {
                if (ImGui::MenuItem("Stop Recording", "R")) {
                    app_->StopRecording();
                }
            } else {
                if (ImGui::MenuItem("Start Recording", "R")) {
                    app_->StartRecording("output.mp4");
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Recording Settings...")) {
                // TODO: Show recording settings dialog
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Sprites")) {
            if (ImGui::MenuItem("Add Image...")) {
                // TODO: Add image sprite
            }
            if (ImGui::MenuItem("Add GIF...")) {
                // TODO: Add GIF sprite
            }
            if (ImGui::MenuItem("Add Video...")) {
                // TODO: Add video sprite
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Clear All Sprites")) {
                // TODO: Clear all sprites
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Fullscreen", "F11")) {
                // TODO: Toggle fullscreen
            }
            ImGui::Separator();
            ImGui::MenuItem("Control Panel", nullptr, true);
            ImGui::MenuItem("Preset Manager", nullptr, true);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                showAbout_ = true;
            }
            if (ImGui::MenuItem("Documentation")) {
                // TODO: Open documentation
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    // About dialog
    if (showAbout_) {
        ImGui::OpenPopup("About");
        showAbout_ = false;
    }

    if (ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Audio Visualizer");
        ImGui::Text("Version 1.0.0");
        ImGui::Separator();
        ImGui::Text("A powerful audio visualization tool");
        ImGui::Text("Powered by ProjectM and FFmpeg");
        ImGui::Separator();
        if (ImGui::Button("Close", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

} // namespace UI
} // namespace AudioVisualizer
