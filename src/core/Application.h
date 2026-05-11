#ifndef AUDIOVISUALIZER_APPLICATION_H
#define AUDIOVISUALIZER_APPLICATION_H

#include "utils/Config.h"
#include <memory>

// Forward declarations
struct GLFWwindow;

namespace AudioVisualizer {
    namespace Core {
        class AudioEngine;
        class VisualizationEngine;
#ifdef HAVE_FFMPEG
        class VideoRecorder;
#endif
    }
    namespace Graphics {
        class Renderer;
        class SpriteManager;
        class EffectsProcessor;
        class ShaderManager;
    }
    namespace UI {
        class MainWindow;
        class ControlPanel;
        class PresetManager;
    }
}

namespace AudioVisualizer {
namespace Core {

class Application {
public:
    explicit Application(const Utils::Config& config);
    ~Application();

    // Non-copyable
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    bool Initialize();
    int Run();
    void Shutdown();

    // State management
    void Play();
    void Pause();
    void Stop();
    bool IsPlaying() const { return isPlaying_; }
    
    // Audio file loading
    bool LoadAudioFile(const std::string& filePath);
    
    // Recording
    void StartRecording(const std::string& outputPath);
    void StopRecording();
    bool IsRecording() const { return isRecording_; }

    // Getters
    GLFWwindow* GetWindow() const { return window_; }
    Utils::Config& GetConfig() { return config_; }
    VisualizationEngine* GetVisualizationEngine() const { return visualizationEngine_.get(); }

private:
    void ProcessInput();
    void Update(float deltaTime);
    void Render();
    void RenderUI();

    // Configuration
    Utils::Config config_;

    // Window
    GLFWwindow* window_;
    int windowWidth_;
    int windowHeight_;
    bool isFullscreen_;

    // Core components
    std::unique_ptr<AudioEngine> audioEngine_;
    std::unique_ptr<VisualizationEngine> visualizationEngine_;
#ifdef HAVE_FFMPEG
    std::unique_ptr<VideoRecorder> videoRecorder_;
#endif

    // Graphics components
    std::unique_ptr<Graphics::Renderer> renderer_;
    std::unique_ptr<Graphics::SpriteManager> spriteManager_;
    std::unique_ptr<Graphics::EffectsProcessor> effectsProcessor_;
    std::unique_ptr<Graphics::ShaderManager> shaderManager_;

    // UI components
    std::unique_ptr<UI::MainWindow> mainWindow_;
    std::unique_ptr<UI::ControlPanel> controlPanel_;
    std::unique_ptr<UI::PresetManager> presetManager_;

    // State
    bool isRunning_;
    bool isPlaying_;
    bool isRecording_;
    double lastFrameTime_;
    float fps_;
};

} // namespace Core
} // namespace AudioVisualizer

#endif // AUDIOVISUALIZER_APPLICATION_H
