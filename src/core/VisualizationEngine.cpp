#include "VisualizationEngine.h"
#include "utils/Logger.h"
#include "graphics/ShaderManager.h"
#include <GL/glew.h>
#include <algorithm>
#include <cmath>

// Note: ProjectM integration will be completed once the library is properly linked
// This is a stub implementation that shows the structure

namespace AudioVisualizer {
namespace Core {

VisualizationEngine::VisualizationEngine(const Utils::Config& config)
    : config_(config)
#ifdef HAVE_PROJECTM
    , projectM_(nullptr)
#else
    , projectM_(nullptr)
#endif
    , width_(1920)
    , height_(1080)
    , textureWidth_(1024)
    , textureHeight_(1024)
    , fps_(60)
    , presetDuration_(30.0f)
    , smoothTransition_(true)
    , beatSensitivity_(1.0f)
    , aspectCorrection_(true)
    , currentPresetIndex_(0)
    , vizMode_(VisualizationMode::Bars)
    , colorScheme_(ColorScheme::Rainbow)
    , timeSinceLastBeat_(0.0f)
    , rotation_(0.0f)
    , shaderManager_(nullptr)
    , elapsedTime_(0.0f)
    , smoothedBass_(0.0f)
    , smoothedMid_(0.0f)
    , smoothedTreble_(0.0f)
    , isInitialized_(false)
{
    width_ = config_.GetInt("visualization.width", 1920);
    height_ = config_.GetInt("visualization.height", 1080);
    fps_ = config_.GetInt("visualization.fps", 60);
    
    // PCM buffer size for ProjectM (typically 512 or 1024 samples per channel)
    pcmData_.resize(1024 * 2); // Stereo
}

VisualizationEngine::~VisualizationEngine() {
    Shutdown();
}

bool VisualizationEngine::Initialize() {
    Utils::Logger::Info("Initializing visualization engine...");

    try {
        // ProjectM initialization configuration
        // Note: This requires ProjectM 4.0+ API
        // When linking ProjectM, uncomment and configure:
        
        /*
        projectM::Settings settings;
        settings.meshX = 32;
        settings.meshY = 24;
        settings.fps = fps_;
        settings.textureSize = textureWidth_;
        settings.windowWidth = width_;
        settings.windowHeight = height_;
        settings.presetURL = config_.GetString("visualization.preset_path", "assets/presets/");
        settings.smoothPresetDuration = 5.0;
        settings.presetDuration = presetDuration_;
        settings.beatSensitivity = beatSensitivity_;
        settings.aspectCorrection = aspectCorrection_;
        settings.easterEgg = 0.0f;
        settings.shuffleEnabled = true;
        
        projectM_ = std::make_unique<libprojectM::ProjectM>(settings);
        
        Utils::Logger::Info("ProjectM initialized with " + 
                          std::to_string(GetPresetCount()) + " presets");
        */

        // Placeholder initialization
        Utils::Logger::Info("Visualization engine initialized (ProjectM stub)");
        
        // Initialize smoothed spectrum buffer
        smoothedSpectrum_.resize(1024, 0.0f);
        
        // Reserve space for particles
        particles_.reserve(1000);
        
        isInitialized_ = true;
        return true;

    } catch (const std::exception& e) {
        Utils::Logger::Error("Failed to initialize visualization engine: " + std::string(e.what()));
        return false;
    }
}

void VisualizationEngine::Shutdown() {
    if (!isInitialized_) {
        return;
    }

#ifdef HAVE_PROJECTM
    projectM_.reset();
#endif
    isInitialized_ = false;
    Utils::Logger::Info("Visualization engine shut down");
}

void VisualizationEngine::Resize(int width, int height) {
    width_ = width;
    height_ = height;
}

void VisualizationEngine::Update(const AudioData& audioData, float deltaTime) {
    if (!isInitialized_) {
        return;
    }

    // Store current audio data for rendering
    currentAudioData_ = audioData;
    
    // Minimal smoothing for maximum responsiveness
    for (size_t i = 0; i < std::min(audioData.spectrum.size(), smoothedSpectrum_.size()); ++i) {
        smoothedSpectrum_[i] = smoothedSpectrum_[i] * 0.05f + audioData.spectrum[i] * 0.95f;
    }
    
    // Update rotation for animated visualizations
    rotation_ += deltaTime * 120.0f;  // 120 degrees per second (super fast)
    if (rotation_ > 360.0f) rotation_ -= 360.0f;
    
    // Update elapsed time for shaders
    elapsedTime_ += deltaTime;
    
    // Calculate frequency bands for shader use
    float bass = 0.0f, mid = 0.0f, treble = 0.0f;
    size_t specSize = smoothedSpectrum_.size();
    if (specSize > 0) {
        // Bass: 0-250 Hz (first 12% of spectrum)
        size_t bassEnd = specSize * 12 / 100;
        for (size_t i = 0; i < bassEnd; ++i) {
            bass += smoothedSpectrum_[i];
        }
        bass /= bassEnd;
        
        // Mid: 250 Hz - 4kHz (12% - 40% of spectrum)
        size_t midStart = bassEnd;
        size_t midEnd = specSize * 40 / 100;
        for (size_t i = midStart; i < midEnd; ++i) {
            mid += smoothedSpectrum_[i];
        }
        mid /= (midEnd - midStart);
        
        // Treble: 4kHz+ (40%+ of spectrum)
        for (size_t i = midEnd; i < specSize; ++i) {
            treble += smoothedSpectrum_[i];
        }
        treble /= (specSize - midEnd);
    }
    
    // Apply additional smoothing for shaders (prevents jitter/stutter, creates gradual transitions)
    // Extra heavy smoothing for smooth color transitions and gentle rotation changes
    smoothedBass_ = smoothedBass_ * 0.94f + bass * 0.06f;
    smoothedMid_ = smoothedMid_ * 0.90f + mid * 0.10f;
    smoothedTreble_ = smoothedTreble_ * 0.88f + treble * 0.12f;
    
    // Update particles
    UpdateParticles(deltaTime);
    
    // Spawn particles on beat
    if (audioData.beat > 0.5f) {
        if (timeSinceLastBeat_ > 0.1f) {  // Spawn more frequently
            SpawnParticlesForBeat();
            timeSinceLastBeat_ = 0.0f;
        }
    }
    timeSinceLastBeat_ += deltaTime;
    
    // Debug logging for first update with audio
    static bool firstUpdate = true;
    if (firstUpdate && audioData.volume > 0.01f) {
        Utils::Logger::Info("Receiving audio data - Volume: " + std::to_string(audioData.volume) + 
                          ", Spectrum size: " + std::to_string(audioData.spectrum.size()));
        firstUpdate = false;
    }

    // Convert audio data to ProjectM format
    ConvertAudioData(audioData);

    // Update ProjectM
    if (projectM_) {
        // projectM_->pcm()->addPCMfloat(pcmData_.data(), pcmData_.size() / 2);
    }
}

void VisualizationEngine::Render() {
    if (!isInitialized_) {
        return;
    }

#ifdef HAVE_PROJECTM
    if (projectM_) {
        // Render ProjectM visualization
        // projectM_->renderFrame();
    } else {
        RenderFallbackVisualization();
    }
#else
    RenderFallbackVisualization();
#endif
}

void VisualizationEngine::RenderFallbackVisualization() {
    // Save current OpenGL state
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width_, 0, height_, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Disable depth test for 2D rendering
    glDisable(GL_DEPTH_TEST);
    
    // Enable blending for particles and effects
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Clear with dark background
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Log first render for debugging
    static bool firstRender = true;
    if (firstRender) {
        Utils::Logger::Info("Rendering fallback visualization");
        firstRender = false;
    }

    // Render based on current visualization mode
    switch (vizMode_) {
        case VisualizationMode::Bars:
            RenderBars();
            break;
        case VisualizationMode::CircularSpectrum:
            RenderCircularSpectrum();
            break;
        case VisualizationMode::Waveform:
            RenderWaveform();
            break;
        case VisualizationMode::Particles:
            RenderParticles();
            break;
        case VisualizationMode::RadialBars:
            RenderRadialBars();
            break;
        case VisualizationMode::MirroredBars:
            RenderMirroredBars();
            break;
        case VisualizationMode::ShaderPlasma:
        case VisualizationMode::ShaderTunnel:
        case VisualizationMode::ShaderFractal:
        case VisualizationMode::ShaderKaleidoscope:
        case VisualizationMode::ShaderWave:
            RenderShaderVisualization();
            break;
    }

    // Draw volume level indicator
    float volumeWidth = width_ * currentAudioData_.volume;
    glColor3f(0.2f, 1.0f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(0, height_ - 10);
    glVertex2f(volumeWidth, height_ - 10);
    glVertex2f(volumeWidth, height_ - 5);
    glVertex2f(0, height_ - 5);
    glEnd();
    
    // Restore OpenGL state
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopAttrib();
}

void VisualizationEngine::ConvertAudioData(const AudioData& audioData) {
    // Convert AudioEngine's format to ProjectM's PCM format
    // ProjectM expects interleaved stereo float data [-1, 1]
    
    const size_t waveformSize = audioData.waveform.size();
    const size_t pcmChannelSize = pcmData_.size() / 2;
    
    for (size_t i = 0; i < pcmChannelSize && i < waveformSize; ++i) {
        // Mono to stereo (duplicate to both channels)
        pcmData_[i * 2] = audioData.waveform[i];
        pcmData_[i * 2 + 1] = audioData.waveform[i];
    }
    
    // Pad remaining with zeros if needed
    for (size_t i = waveformSize; i < pcmChannelSize; ++i) {
        pcmData_[i * 2] = 0.0f;
        pcmData_[i * 2 + 1] = 0.0f;
    }
}

void VisualizationEngine::LoadPreset(const std::string& presetPath) {
    if (projectM_) {
        Utils::Logger::Info("Loading preset: " + presetPath);
        // projectM_->selectPreset(presetPath);
    }
}

void VisualizationEngine::LoadPresetByIndex(int index) {
    if (projectM_ && index >= 0 && index < GetPresetCount()) {
        currentPresetIndex_ = index;
        Utils::Logger::Info("Loading preset index: " + std::to_string(index));
        // projectM_->selectPreset(index);
    }
}

void VisualizationEngine::NextPreset() {
    if (projectM_) {
        currentPresetIndex_ = (currentPresetIndex_ + 1) % GetPresetCount();
        LoadPresetByIndex(currentPresetIndex_);
    }
}

void VisualizationEngine::PreviousPreset() {
    if (projectM_) {
        currentPresetIndex_ = (currentPresetIndex_ - 1 + GetPresetCount()) % GetPresetCount();
        LoadPresetByIndex(currentPresetIndex_);
    }
}

void VisualizationEngine::RandomPreset() {
    if (projectM_) {
        int randomIndex = rand() % GetPresetCount();
        LoadPresetByIndex(randomIndex);
    }
}

int VisualizationEngine::GetPresetCount() const {
    if (projectM_) {
        // return projectM_->getPlaylistSize();
        return 0; // Placeholder
    }
    return 0;
}

std::string VisualizationEngine::GetCurrentPresetName() const {
    if (projectM_) {
        // return projectM_->getPresetName(currentPresetIndex_);
        return "Default Preset"; // Placeholder
    }
    return "";
}

std::vector<std::string> VisualizationEngine::GetPresetList() const {
    std::vector<std::string> presets;
    
    if (projectM_) {
        int count = GetPresetCount();
        for (int i = 0; i < count; ++i) {
            // presets.push_back(projectM_->getPresetName(i));
        }
    }
    
    return presets;
}

void VisualizationEngine::SetResolution(int width, int height) {
    width_ = width;
    height_ = height;
    
    if (projectM_) {
        // projectM_->projectM_resetGL(width, height);
    }
}

void VisualizationEngine::SetTextureSize(int width, int height) {
    textureWidth_ = width;
    textureHeight_ = height;
}

void VisualizationEngine::SetFPS(int fps) {
    fps_ = fps;
}

void VisualizationEngine::SetPresetDuration(float seconds) {
    presetDuration_ = seconds;
    
    if (projectM_) {
        // projectM_->settings().presetDuration = seconds;
    }
}

void VisualizationEngine::SetSmoothPresetTransition(bool enabled) {
    smoothTransition_ = enabled;
}

void VisualizationEngine::SetBeatSensitivity(float sensitivity) {
    beatSensitivity_ = std::max(0.0f, std::min(sensitivity, 2.0f));
    
    if (projectM_) {
        // projectM_->settings().beatSensitivity = beatSensitivity_;
    }
}

void VisualizationEngine::SetAspectCorrection(bool enabled) {
    aspectCorrection_ = enabled;
    
    if (projectM_) {
        // projectM_->settings().aspectCorrection = enabled;
    }
}

void VisualizationEngine::SetCustomFloat(const std::string& name, float value) {
    Utils::Logger::Info("Setting custom float '" + name + "' to " + std::to_string(value));
    // Custom parameter handling
}

void VisualizationEngine::SetCustomInt(const std::string& name, int value) {
    Utils::Logger::Info("Setting custom int '" + name + "' to " + std::to_string(value));
    // Custom parameter handling
}

void VisualizationEngine::SetVisualizationMode(VisualizationMode mode) {
    vizMode_ = mode;
}

void VisualizationEngine::SetColorScheme(ColorScheme scheme) {
    colorScheme_ = scheme;
}

void VisualizationEngine::GetColor(float value, float& r, float& g, float& b) {
    // Apply beat boost
    float beatBoost = 1.0f + currentAudioData_.beat * 0.5f;
    
    switch (colorScheme_) {
        case ColorScheme::Rainbow:
            r = (0.5f + 0.5f * std::sin(value * 6.28f)) * beatBoost;
            g = (0.5f + 0.5f * std::sin(value * 6.28f + 2.09f)) * beatBoost;
            b = (0.5f + 0.5f * std::sin(value * 6.28f + 4.19f)) * beatBoost;
            break;
        case ColorScheme::Fire:
            r = 1.0f * beatBoost;
            g = value * 0.8f * beatBoost;
            b = value * value * 0.3f * beatBoost;
            break;
        case ColorScheme::Ocean:
            r = value * 0.3f * beatBoost;
            g = (0.5f + value * 0.5f) * beatBoost;
            b = 1.0f * beatBoost;
            break;
        case ColorScheme::Neon:
            r = (value > 0.5f ? 1.0f : 0.0f) * beatBoost;
            g = (1.0f - value) * beatBoost;
            b = (value < 0.5f ? 1.0f : 0.0f) * beatBoost;
            break;
        case ColorScheme::Monochrome:
            r = g = b = value * beatBoost;
            break;
        case ColorScheme::Purple:
            r = (0.6f + value * 0.4f) * beatBoost;
            g = value * 0.3f * beatBoost;
            b = 1.0f * beatBoost;
            break;
    }
    
    // Clamp values
    r = std::min(1.0f, std::max(0.0f, r));
    g = std::min(1.0f, std::max(0.0f, g));
    b = std::min(1.0f, std::max(0.0f, b));
}

void VisualizationEngine::RenderBars() {
    const size_t numBars = std::min(smoothedSpectrum_.size(), size_t(128));
    if (numBars == 0) return;
    
    const float barWidth = static_cast<float>(width_) / numBars;
    const float maxHeight = static_cast<float>(height_) * 10.0f;  // 5x dramatic

    for (size_t i = 0; i < numBars; ++i) {
        float magnitude = smoothedSpectrum_[i];
        float barHeight = magnitude * maxHeight;
        float value = static_cast<float>(i) / numBars;
        
        float r, g, b;
        GetColor(value, r, g, b);

        float x = i * barWidth;
        glColor3f(r, g, b);
        glBegin(GL_QUADS);
        glVertex2f(x, 0);
        glVertex2f(x + barWidth * 0.8f, 0);
        glVertex2f(x + barWidth * 0.8f, barHeight);
        glVertex2f(x, barHeight);
        glEnd();
    }
}

void VisualizationEngine::RenderCircularSpectrum() {
    const size_t numBars = std::min(smoothedSpectrum_.size(), size_t(128));
    if (numBars == 0) return;
    
    float centerX = width_ * 0.5f;
    float centerY = height_ * 0.5f;
    float baseRadius = std::min(width_, height_) * 0.05f;  // Very small base
    float maxLength = std::min(width_, height_) * 2.8f;  // Much longer bars
    
    // Add beat-responsive pulsing
    float beatScale = 1.0f + (currentAudioData_.beat * 0.3f);
    
    float angleStep = 360.0f / numBars;
    
    for (size_t i = 0; i < numBars; ++i) {
        float magnitude = smoothedSpectrum_[i];
        float barLength = magnitude * maxLength * beatScale;  // Beat-responsive
        float angle = (rotation_ + i * angleStep) * 3.14159f / 180.0f;
        
        float innerX = centerX + baseRadius * std::cos(angle);
        float innerY = centerY + baseRadius * std::sin(angle);
        float outerX = centerX + (baseRadius + barLength) * std::cos(angle);
        float outerY = centerY + (baseRadius + barLength) * std::sin(angle);
        
        float value = static_cast<float>(i) / numBars;
        float r, g, b;
        GetColor(value, r, g, b);
        
        glLineWidth(3.0f);
        glBegin(GL_LINES);
        glColor3f(r, g, b);
        glVertex2f(innerX, innerY);
        glColor3f(r * 0.5f, g * 0.5f, b * 0.5f);
        glVertex2f(outerX, outerY);
        glEnd();
    }
}

void VisualizationEngine::RenderWaveform() {
    const size_t numSamples = std::min(currentAudioData_.waveform.size(), size_t(width_));
    if (numSamples == 0) return;
    
    float centerY = height_ * 0.5f;
    float scale = height_ * 0.5f;  // Reduced waveform amplitude
    
    glLineWidth(2.0f);
    glBegin(GL_LINE_STRIP);
    
    for (size_t i = 0; i < numSamples; ++i) {
        float sample = currentAudioData_.waveform[i];
        float x = (static_cast<float>(i) / numSamples) * width_;
        float y = centerY + sample * scale;
        
        float value = std::abs(sample);
        float r, g, b;
        GetColor(value, r, g, b);
        glColor3f(r, g, b);
        glVertex2f(x, y);
    }
    
    glEnd();
}

void VisualizationEngine::RenderParticles() {
    // Draw particles with larger size and glow effect
    glPointSize(8.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // Additive blending for glow
    glBegin(GL_POINTS);
    
    for (const auto& p : particles_) {
        // Boost alpha for visibility
        glColor4f(p.r * 1.5f, p.g * 1.5f, p.b * 1.5f, p.life * 0.9f);
        glVertex2f(p.x, p.y);
    }
    
    glEnd();
    glDisable(GL_BLEND);
    
    // Also render bars in background
    RenderBars();
}

void VisualizationEngine::RenderRadialBars() {
    const size_t numBars = std::min(smoothedSpectrum_.size(), size_t(64));
    if (numBars == 0) return;
    
    float centerX = width_ * 0.5f;
    float centerY = height_ * 0.5f;
    float angleStep = 360.0f / numBars;
    float maxLength = std::min(width_, height_) * 2.2f;  // Much longer radial bars
    
    // Add beat-responsive pulsing and faster rotation
    float beatScale = 1.0f + (currentAudioData_.beat * 0.4f);
    float dynamicRotation = rotation_ * 1.5f;  // 50% faster rotation
    
    for (size_t i = 0; i < numBars; ++i) {
        float magnitude = smoothedSpectrum_[i];
        float barLength = magnitude * maxLength * beatScale;  // Beat-responsive
        float angle1 = (dynamicRotation + i * angleStep) * 3.14159f / 180.0f;
        float angle2 = (dynamicRotation + (i + 1) * angleStep) * 3.14159f / 180.0f;
        
        float value = static_cast<float>(i) / numBars;
        float r, g, b;
        GetColor(value, r, g, b);
        
        glBegin(GL_TRIANGLE_FAN);
        glColor3f(r * 0.3f, g * 0.3f, b * 0.3f);
        glVertex2f(centerX, centerY);
        glColor3f(r, g, b);
        glVertex2f(centerX + barLength * std::cos(angle1), centerY + barLength * std::sin(angle1));
        glVertex2f(centerX + barLength * std::cos(angle2), centerY + barLength * std::sin(angle2));
        glEnd();
    }
}

void VisualizationEngine::RenderMirroredBars() {
    const size_t numBars = std::min(smoothedSpectrum_.size(), size_t(64));
    if (numBars == 0) return;
    
    const float barWidth = static_cast<float>(width_) / numBars;
    const float maxHeight = static_cast<float>(height_) * 1.3f;  // Mirrored bars (5x)
    const float centerY = height_ * 0.5f;

    for (size_t i = 0; i < numBars; ++i) {
        float magnitude = smoothedSpectrum_[i];
        float barHeight = magnitude * maxHeight;
        float value = static_cast<float>(i) / numBars;
        
        float r, g, b;
        GetColor(value, r, g, b);

        float x = i * barWidth;
        
        // Top half
        glColor3f(r, g, b);
        glBegin(GL_QUADS);
        glVertex2f(x, centerY);
        glVertex2f(x + barWidth * 0.8f, centerY);
        glVertex2f(x + barWidth * 0.8f, centerY + barHeight);
        glVertex2f(x, centerY + barHeight);
        glEnd();
        
        // Bottom half (mirrored)
        glBegin(GL_QUADS);
        glVertex2f(x, centerY);
        glVertex2f(x + barWidth * 0.8f, centerY);
        glVertex2f(x + barWidth * 0.8f, centerY - barHeight);
        glVertex2f(x, centerY - barHeight);
        glEnd();
    }
}

void VisualizationEngine::UpdateParticles(float deltaTime) {
    // Update existing particles
    for (auto it = particles_.begin(); it != particles_.end();) {
        it->x += it->vx * deltaTime;
        it->y += it->vy * deltaTime;
        it->life -= deltaTime * 0.25f;  // Fade slower for longer visibility
        
        // Gravity
        it->vy -= 100.0f * deltaTime;
        
        if (it->life <= 0.0f) {
            it = particles_.erase(it);
        } else {
            ++it;
        }
    }
}

void VisualizationEngine::SpawnParticlesForBeat() {
    const int numParticles = 80;  // Much more particles
    float centerX = width_ * 0.5f;
    float centerY = height_ * 0.5f;
    
    for (int i = 0; i < numParticles; ++i) {
        Particle p;
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = 150.0f + (rand() % 300);  // Faster particles
        
        p.x = centerX;
        p.y = centerY;
        p.vx = std::cos(angle) * speed;
        p.vy = std::sin(angle) * speed;
        p.life = 1.0f;
        p.size = 2.0f + (rand() % 4);
        
        float value = static_cast<float>(rand() % 100) / 100.0f;
        GetColor(value, p.r, p.g, p.b);
        
        particles_.push_back(p);
    }
}

bool VisualizationEngine::InitializeShaders(Graphics::ShaderManager* shaderManager) {
    if (!shaderManager) {
        Utils::Logger::Error("ShaderManager is null, cannot initialize shader visualizations");
        return false;
    }
    
    shaderManager_ = shaderManager;
    CreateShaderPresets();
    
    Utils::Logger::Info("Shader-based visualizations initialized");
    return true;
}

void VisualizationEngine::CreateShaderPresets() {
    if (!shaderManager_) return;
    
    // Simple vertex shader for all fullscreen effects
    std::string vertexShader = R"(
        #version 330 core
        layout(location = 0) in vec2 position;
        out vec2 fragCoord;
        
        void main() {
            fragCoord = position * 0.5 + 0.5;
            gl_Position = vec4(position, 0.0, 1.0);
        }
    )";
    
    // Plasma shader - highly reactive to music
    std::string plasmaShader = R"(
        #version 330 core
        in vec2 fragCoord;
        out vec4 fragColor;
        
        uniform float time;
        uniform float bass;
        uniform float mid;
        uniform float treble;
        uniform vec2 resolution;
        
        void main() {
            // Scale changes with bass - more subtle variation
            float scale = 10.0 + bass * 5.0;
            vec2 uv = fragCoord * scale;
            
            // Speed changes with mid frequencies - slower and smoother
            float speed = 0.5 + mid * 1.0;
            
            // Multiple plasma waves with gentler movement
            float v1 = sin(uv.x + time * speed * 0.5);
            float v2 = sin(uv.y + time * speed * 0.35);
            float v3 = sin((uv.x + uv.y) * 0.5 + time * speed * 0.25);
            float v4 = sin(sqrt(uv.x*uv.x + uv.y*uv.y) + time * speed * 0.2);
            
            // Treble adds complexity/distortion - more subtle
            float v5 = sin(uv.x * 2.0 + time * treble * 2.0) * treble * 0.5;
            float v6 = cos(uv.y * 2.0 - time * treble * 2.0) * treble * 0.5;
            
            float v = (v1 + v2 + v3 + v4 + v5 + v6) / 6.0;
            
            // Color shifting based on audio - more gradual
            float colorShift = time * 0.5 + bass * 5.0;
            vec3 col = vec3(
                sin(v * 3.14159 * 2.0 + colorShift) * 0.5 + 0.5,
                sin(v * 3.14159 * 2.0 + colorShift + mid * 5.0 + 2.094) * 0.5 + 0.5,
                sin(v * 3.14159 * 2.0 + colorShift + treble * 8.0 + 4.189) * 0.5 + 0.5
            );
            
            // Brightness pulses with overall energy
            float energy = (bass + mid + treble) / 3.0;
            col *= 0.7 + energy * 0.8;
            
            fragColor = vec4(col, 1.0);
        }
    )";
    
    // Tunnel shader - smooth audio-reactive tunnel effect
    std::string tunnelShader = R"(
        #version 330 core
        in vec2 fragCoord;
        out vec4 fragColor;
        
        uniform float time;
        uniform float bass;
        uniform float mid;
        uniform float treble;
        uniform vec2 resolution;
        
        void main() {
            vec2 uv = (fragCoord - 0.5) * 2.0;
            uv.x *= resolution.x / resolution.y;
            
            float r = length(uv);
            float a = atan(uv.y, uv.x);
            
            // Bass controls tunnel speed and depth - slower and more gradual
            float tunnelSpeed = 0.2 + bass * 0.3;
            float tunnelDepth = 1.0 / (r + 0.15);
            float tunnel = tunnelDepth + time * tunnelSpeed;
            
            // Mid controls spiral intensity - more subtle changes
            float spiralIntensity = 4.0 + mid * 4.0;
            float spiral = a * spiralIntensity / 3.14159 + time * 0.2;
            
            // Treble adds pulsing rings - more subtle
            float rings = sin(tunnelDepth * 10.0 - time * 1.5 + treble * 3.0) * treble * 0.2;
            
            // Combine effects
            float pattern = tunnel * 3.0 + spiral + rings;
            
            // Smooth color cycling with audio influence - slower transitions
            vec3 col = vec3(
                sin(pattern + time * 0.3 + bass * 1.5) * 0.5 + 0.5,
                sin(pattern + time * 0.3 + mid * 1.5 + 2.094) * 0.5 + 0.5,
                sin(pattern + time * 0.3 + treble * 1.5 + 4.189) * 0.5 + 0.5
            );
            
            // Vignette effect (darker at edges) with bass-reactive brightness - more subtle
            float vignette = 1.0 - r * 0.4;
            float brightness = 0.85 + bass * 0.25;
            col *= vignette * brightness;
            
            // Add glow in the center
            col += vec3(0.1, 0.05, 0.15) / (r + 0.3);
            
            fragColor = vec4(col, 1.0);
        }
    )";
    
    // Fractal shader - smooth Julia set with music reactivity
    std::string fractalShader = R"(
        #version 330 core
        in vec2 fragCoord;
        out vec4 fragColor;
        
        uniform float time;
        uniform float bass;
        uniform float mid;
        uniform float treble;
        uniform vec2 resolution;
        
        void main() {
            // Bass controls zoom level - zoomed out more to show more fractals
            float zoom = 3.5 + bass * 2.0;
            vec2 uv = (fragCoord - 0.5) * zoom;
            uv.x *= resolution.x / resolution.y;
            
            // Mid controls rotation
            float angle = time * 0.15 + mid * 0.5;
            float s = sin(angle);
            float c = cos(angle);
            uv = vec2(uv.x * c - uv.y * s, uv.x * s + uv.y * c);
            
            vec2 z = uv;
            
            // Fractal parameters change slowly with music
            float cSpeed = time * 0.1;
            vec2 cBase = vec2(sin(cSpeed) * 0.4, cos(cSpeed * 0.7) * 0.4);
            vec2 cOffset = vec2(bass * 0.3, mid * 0.3);
            vec2 cParam = cBase + cOffset;
            
            float iter = 0.0;
            // Increased iterations for more detail
            for (int i = 0; i < 80; i++) {
                z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + cParam;
                if (length(z) > 2.0) break;
                iter += 1.0;
            }
            
            // Smooth color based on iteration count and audio
            float t = iter * 0.05 + time * 0.2;
            float energy = (bass + mid + treble) / 3.0;
            
            vec3 col = vec3(
                sin(t + treble * 1.5) * 0.5 + 0.5,
                sin(t * 0.7 + mid * 1.5 + 2.094) * 0.5 + 0.5,
                sin(t * 0.4 + bass * 1.5 + 4.189) * 0.5 + 0.5
            );
            
            // Improved brightness - never too dark, always visible
            float fade = iter / 80.0;
            // Ensure minimum brightness of 0.4, boost with energy
            fade = max(fade, 0.4) * (0.8 + energy * 0.6);
            
            // Add ambient glow to prevent total darkness
            col = col * fade + vec3(0.15, 0.1, 0.2) * (1.0 - fade * 0.5);
            
            fragColor = vec4(col, 1.0);
        }
    )";
    
    // Kaleidoscope shader - smooth symmetrical patterns with music reactivity
    std::string kaleidoscopeShader = R"(
        #version 330 core
        in vec2 fragCoord;
        out vec4 fragColor;
        
        uniform float time;
        uniform float bass;
        uniform float mid;
        uniform float treble;
        uniform vec2 resolution;
        
        void main() {
            vec2 uv = (fragCoord - 0.5) * 2.0;
            uv.x *= resolution.x / resolution.y;
            
            float r = length(uv);
            float a = atan(uv.y, uv.x);
            
            // Fixed number of segments - no audio influence to prevent stuttering
            float segments = 8.0;
            float segmentAngle = 3.14159 * 2.0 / segments;
            a = mod(a, segmentAngle);
            a = abs(a - segmentAngle * 0.5);
            
            // Smooth rotation: constant base + gentle audio influence
            float rotation = time * 0.15 + mid * 0.3;
            float s = sin(rotation);
            float c = cos(rotation);
            
            vec2 p = vec2(cos(a), sin(a)) * r;
            p = vec2(p.x * c - p.y * s, p.x * s + p.y * c);
            
            // Mid controls pattern complexity/scale (this won't cause rotation jitter)
            float patternScale = 8.0 + mid * 4.0;
            
            // Multiple pattern layers - bass affects pattern depth/intensity, not geometry
            float pattern1 = sin(p.x * patternScale + time * 0.5) * sin(p.y * patternScale + time * 0.4);
            float pattern2 = sin(length(p) * (15.0 + bass * 5.0) - time * 0.3) * 0.5;
            float pattern3 = sin(p.x * 5.0 - p.y * 5.0 + treble * 4.0) * 0.3;
            
            float pattern = pattern1 + pattern2 + pattern3;
            
            // Energy-based brightness
            float energy = (bass + mid + treble) / 3.0;
            
            // Smooth color shifting - audio affects intensity and hue
            float colorIntensity = 0.4 + energy * 0.6;
            // Bass affects hue shift (smoothed values prevent jitter)
            float hueShift = bass * 1.2;
            
            // Smooth color cycling with audio-driven intensity
            vec3 col = vec3(
                sin(pattern * 4.0 + time * 0.3 + hueShift) * colorIntensity + 0.5,
                sin(pattern * 5.0 + time * 0.3 + hueShift + 2.094) * colorIntensity + 0.5,
                sin(pattern * 6.0 + time * 0.3 + hueShift + 4.189) * colorIntensity + 0.5
            );
            
            // Vignette with energy boost
            float vignette = 1.0 - r * 0.25;
            col *= vignette * (0.8 + energy * 0.4);
            
            // Add center glow
            col += vec3(0.2, 0.15, 0.25) / (r * 2.0 + 0.5);
            
            fragColor = vec4(col, 1.0);
        }
    )";
    
    // Wave shader - smooth flowing waves with music reactivity
    std::string waveShader = R"(
        #version 330 core
        in vec2 fragCoord;
        out vec4 fragColor;
        
        uniform float time;
        uniform float bass;
        uniform float mid;
        uniform float treble;
        uniform vec2 resolution;
        
        void main() {
            vec2 uv = fragCoord;
            
            // Bass controls wave amplitude
            float amplitude = 0.08 + bass * 0.12;
            
            // Mid controls wave frequency
            float frequency = 8.0 + mid * 4.0;
            
            // Treble adds twist/distortion
            float twist = treble * 0.4;
            
            // Add dynamic vertical stretching based on position
            float stretch = sin(uv.x * 3.0 + time * 0.5) * 0.15 + cos(uv.x * 5.0 - time * 0.3) * 0.1;
            
            // Multiple smooth wave layers with phase shifts
            float wave1 = sin(uv.x * frequency + time * 1.0 + uv.y * twist * 5.0) * amplitude;
            float wave2 = sin(uv.x * frequency * 0.7 + time * 0.8 + uv.y * twist * 3.0) * amplitude * 0.7;
            float wave3 = sin(uv.x * frequency * 1.3 - time * 1.2 - uv.y * twist * 4.0) * amplitude * 0.5;
            
            // Treble adds high-frequency ripples with twist
            float ripple = sin(uv.x * frequency * 3.0 + time * 2.0 + treble * 3.0 + uv.y * 2.0) * treble * 0.03;
            
            // Create multiple wave lines at different heights with stretch
            float wave = wave1 + wave2 + wave3 + ripple;
            
            // Three wave lines with dynamic stretching
            float pos1 = 0.3 + stretch * 0.5;
            float pos2 = 0.5 + stretch * 0.3;
            float pos3 = 0.7 + stretch * 0.2;
            
            float d1 = abs(uv.y - pos1 - wave);
            float d2 = abs(uv.y - pos2 - wave * 0.8);
            float d3 = abs(uv.y - pos3 - wave * 0.6);
            
            // Smooth line rendering with glow
            float line1 = smoothstep(0.025, 0.0, d1);
            float line2 = smoothstep(0.020, 0.0, d2);
            float line3 = smoothstep(0.015, 0.0, d3);
            
            // Add glow/bloom effect
            float glow1 = exp(-d1 * 15.0) * 0.4;
            float glow2 = exp(-d2 * 15.0) * 0.3;
            float glow3 = exp(-d3 * 15.0) * 0.2;
            
            float totalLine = line1 + line2 + line3;
            float totalGlow = glow1 + glow2 + glow3;
            
            // Energy-based brightness
            float energy = (bass + mid + treble) / 3.0;
            
            // Smooth color cycling with audio influence
            float colorPhase = time * 0.3;
            vec3 col = vec3(
                (sin(colorPhase + bass * 1.0) * 0.5 + 0.5),
                (sin(colorPhase + mid * 1.0 + 2.094) * 0.5 + 0.5),
                (sin(colorPhase + treble * 1.0 + 4.189) * 0.5 + 0.5)
            );
            
            // Apply colors to lines and glow
            col = col * (totalLine * 1.2 + totalGlow * 0.8) * (0.8 + energy * 0.4);
            
            // Subtle background gradient
            col += vec3(0.02, 0.03, 0.05) * (1.0 - uv.y);
            
            fragColor = vec4(col, 1.0);
        }
    )";
    
    // Load shaders
    shaderManager_->LoadShaderFromSource("plasma", vertexShader, plasmaShader);
    shaderManager_->LoadShaderFromSource("tunnel", vertexShader, tunnelShader);
    shaderManager_->LoadShaderFromSource("fractal", vertexShader, fractalShader);
    shaderManager_->LoadShaderFromSource("kaleidoscope", vertexShader, kaleidoscopeShader);
    shaderManager_->LoadShaderFromSource("wave", vertexShader, waveShader);
    
    Utils::Logger::Info("Created 5 shader presets");
}

void VisualizationEngine::RenderShaderVisualization() {
    if (!shaderManager_) {
        // Fall back to bars if shaders not available
        RenderBars();
        return;
    }
    
    // Determine which shader to use based on mode
    std::string shaderName;
    switch (vizMode_) {
        case VisualizationMode::ShaderPlasma:
            shaderName = "plasma";
            break;
        case VisualizationMode::ShaderTunnel:
            shaderName = "tunnel";
            break;
        case VisualizationMode::ShaderFractal:
            shaderName = "fractal";
            break;
        case VisualizationMode::ShaderKaleidoscope:
            shaderName = "kaleidoscope";
            break;
        case VisualizationMode::ShaderWave:
            shaderName = "wave";
            break;
        default:
            return;
    }
    
    uint32_t shaderId = shaderManager_->GetShader(shaderName);
    if (shaderId == 0) {
        RenderBars();
        return;
    }
    
    // Use shader and set uniforms with smoothed audio values
    shaderManager_->UseShader(shaderId);
    shaderManager_->SetFloat(shaderId, "time", elapsedTime_);
    // Use pre-smoothed values with 25x amplification for dramatic effect
    shaderManager_->SetFloat(shaderId, "bass", smoothedBass_ * 25.0f);
    shaderManager_->SetFloat(shaderId, "mid", smoothedMid_ * 25.0f);
    shaderManager_->SetFloat(shaderId, "treble", smoothedTreble_ * 25.0f);
    shaderManager_->SetVec2(shaderId, "resolution", static_cast<float>(width_), static_cast<float>(height_));
    
    // Render fullscreen quad
    RenderFullScreenQuad();
    
    // Reset to default shader
    glUseProgram(0);
}

void VisualizationEngine::RenderFullScreenQuad() {
    // Render a fullscreen quad using immediate mode
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(-1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glEnd();
}

} // namespace Core
} // namespace AudioVisualizer
