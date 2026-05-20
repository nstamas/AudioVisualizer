#ifndef AUDIOVISUALIZER_VISUALIZATIONENGINE_H
#define AUDIOVISUALIZER_VISUALIZATIONENGINE_H

#include "AudioEngine.h"
#include "utils/Config.h"
#include <string>
#include <vector>
#include <memory>

// ProjectM is optional - only use if available
#ifdef HAVE_PROJECTM
namespace libprojectM {
    class ProjectM;
}
#endif

namespace AudioVisualizer {
namespace Graphics {
    class ShaderManager;
}

namespace Core {

enum class VisualizationMode {
    Bars,
    CircularSpectrum,
    Waveform,
    Particles,
    RadialBars,
    MirroredBars,
    ShaderPlasma,
    ShaderTunnel,
    ShaderFractal,
    ShaderKaleidoscope,
    ShaderWave
};

enum class ColorScheme {
    Rainbow,
    Fire,
    Ocean,
    Neon,
    Monochrome,
    Purple
};

struct Particle {
    float x, y;
    float vx, vy;
    float life;
    float size;
    float r, g, b;
};

class VisualizationEngine {
public:
    explicit VisualizationEngine(const Utils::Config& config);
    ~VisualizationEngine();

    bool Initialize();
    bool InitializeShaders(Graphics::ShaderManager* shaderManager);
    void Shutdown();
    
    // Resize handling
    void Resize(int width, int height);

    void Update(const AudioData& audioData, float deltaTime);
    void Render();

    // Preset management
    void LoadPreset(const std::string& presetPath);
    void LoadPresetByIndex(int index);
    void NextPreset();
    void PreviousPreset();
    void RandomPreset();
    int GetPresetCount() const;
    std::string GetCurrentPresetName() const;
    std::vector<std::string> GetPresetList() const;

    // Configuration
    void SetResolution(int width, int height);
    void SetTextureSize(int width, int height);
    void SetFPS(int fps);
    void SetPresetDuration(float seconds);
    void SetSmoothPresetTransition(bool enabled);
    
    // Rendering options
    void SetBeatSensitivity(float sensitivity);
    void SetAspectCorrection(bool enabled);
    
    // Custom parameters
    void SetCustomFloat(const std::string& name, float value);
    void SetCustomInt(const std::string& name, int value);
    
    // Visualization mode control
    void SetVisualizationMode(VisualizationMode mode);
    VisualizationMode GetVisualizationMode() const { return vizMode_; }
    
    // Color scheme control
    void SetColorScheme(ColorScheme scheme);
    ColorScheme GetColorScheme() const { return colorScheme_; }

private:
    void ConvertAudioData(const AudioData& audioData);
    void RenderFallbackVisualization();
    
    // Enhanced visualization rendering methods
    void RenderBars();
    void RenderCircularSpectrum();
    void RenderWaveform();
    void RenderParticles();
    void RenderRadialBars();
    void RenderMirroredBars();
    
    // Shader-based visualization methods
    void RenderShaderVisualization();
    void RenderFullScreenQuad();
    void CreateShaderPresets();
    
    // Helper methods
    void GetColor(float value, float& r, float& g, float& b);
    void UpdateParticles(float deltaTime);
    void SpawnParticlesForBeat();

#ifdef HAVE_PROJECTM
    std::unique_ptr<libprojectM::ProjectM> projectM_;
#else
    void* projectM_;  // Placeholder when ProjectM not available
#endif
    Utils::Config config_;
    
    int width_;
    int height_;
    int textureWidth_;
    int textureHeight_;
    int fps_;
    
    float presetDuration_;
    bool smoothTransition_;
    float beatSensitivity_;
    bool aspectCorrection_;
    
    std::vector<float> pcmData_;
    AudioData currentAudioData_;  // Store latest audio data for rendering
    int currentPresetIndex_;
    
    // Visualization mode and effects
    VisualizationMode vizMode_;
    ColorScheme colorScheme_;
    std::vector<Particle> particles_;
    float timeSinceLastBeat_;
    float rotation_;  // For rotating visualizations
    std::vector<float> smoothedSpectrum_;  // Smoothed spectrum for better visuals
    
    // Shader-based visualization
    Graphics::ShaderManager* shaderManager_;
    float elapsedTime_;  // For shader animations
    
    // Smoothed audio values for shaders (prevents jitter)
    float smoothedBass_;
    float smoothedMid_;
    float smoothedTreble_;
    
    bool isInitialized_;
};

} // namespace Core
} // namespace AudioVisualizer

#endif // AUDIOVISUALIZER_VISUALIZATIONENGINE_H
