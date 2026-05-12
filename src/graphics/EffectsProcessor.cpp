#include "EffectsProcessor.h"
#include "utils/Logger.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstring>

namespace AudioVisualizer {
namespace Graphics {

EffectsProcessor::EffectsProcessor()
    : maskingShader_(0)
    , outlineShader_(0)
    , shadowShader_(0)
    , smearingShader_(0)
    , blurShader_(0)
    , bloomShader_(0)
    , chromaticShader_(0)
    , vignetteShader_(0)
    , maskTexture_(0)
    , outlineWidth_(2.0f)
    , shadowBlur_(5.0f)
    , smearingAmount_(0.5f)
    , blurRadius_(5.0f)
    , bloomThreshold_(0.8f)
    , bloomIntensity_(1.0f)
    , vignetteStrength_(0.5f)
    , quadVAO_(0)
    , quadVBO_(0)
    , isInitialized_(false)
{
    outlineColor_[0] = 1.0f;
    outlineColor_[1] = 1.0f;
    outlineColor_[2] = 1.0f;
    outlineColor_[3] = 1.0f;

    shadowOffset_[0] = 5.0f;
    shadowOffset_[1] = -5.0f;

    std::memset(effectsEnabled_, 0, sizeof(effectsEnabled_));
}

EffectsProcessor::~EffectsProcessor() {
    Shutdown();
}

bool EffectsProcessor::Initialize() {
    Utils::Logger::Info("Initializing effects processor...");

    // Create fullscreen quad for post-processing
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO_);
    glGenBuffers(1, &quadVBO_);

    glBindVertexArray(quadVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 
                         (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Initialize shaders
    InitializeShaders();

    isInitialized_ = true;
    Utils::Logger::Info("Effects processor initialized");
    return true;
}

void EffectsProcessor::Shutdown() {
    if (!isInitialized_) {
        return;
    }

    // Delete shaders
    if (maskingShader_) glDeleteProgram(maskingShader_);
    if (outlineShader_) glDeleteProgram(outlineShader_);
    if (shadowShader_) glDeleteProgram(shadowShader_);
    if (smearingShader_) glDeleteProgram(smearingShader_);
    if (blurShader_) glDeleteProgram(blurShader_);
    if (bloomShader_) glDeleteProgram(bloomShader_);
    if (chromaticShader_) glDeleteProgram(chromaticShader_);
    if (vignetteShader_) glDeleteProgram(vignetteShader_);

    // Delete quad
    if (quadVAO_) glDeleteVertexArrays(1, &quadVAO_);
    if (quadVBO_) glDeleteBuffers(1, &quadVBO_);

    isInitialized_ = false;
}

void EffectsProcessor::InitializeShaders() {
    // TODO: Load and compile effect shaders from files
    // This is a placeholder
    Utils::Logger::Info("Loading effect shaders...");
}

void EffectsProcessor::Apply() {
    if (!isInitialized_) {
        return;
    }

    // Apply enabled effects in sequence
    // TODO: Implement proper effect chain
}

void EffectsProcessor::ApplyEffect(EffectType effect, uint32_t inputTexture, uint32_t outputFBO) {
    if (!IsEffectEnabled(effect)) {
        return;
    }

    switch (effect) {
        case EffectType::Masking:
            ApplyMasking(inputTexture, outputFBO);
            break;
        case EffectType::Outline:
            ApplyOutline(inputTexture, outputFBO);
            break;
        case EffectType::Shadow:
            ApplyShadow(inputTexture, outputFBO);
            break;
        case EffectType::Smearing:
            ApplySmearing(inputTexture, outputFBO);
            break;
        case EffectType::Blur:
            ApplyBlur(inputTexture, outputFBO);
            break;
        case EffectType::Bloom:
            ApplyBloom(inputTexture, outputFBO);
            break;
        case EffectType::ChromaticAberration:
            ApplyChromaticAberration(inputTexture, outputFBO);
            break;
        case EffectType::Vignette:
            ApplyVignette(inputTexture, outputFBO);
            break;
        default:
            break;
    }
}

void EffectsProcessor::ApplyMasking(uint32_t inputTexture, uint32_t outputFBO) {
    // TODO: Implement masking effect
}

void EffectsProcessor::ApplyOutline(uint32_t inputTexture, uint32_t outputFBO) {
    // TODO: Implement outline effect using edge detection
}

void EffectsProcessor::ApplyShadow(uint32_t inputTexture, uint32_t outputFBO) {
    // TODO: Implement shadow effect
}

void EffectsProcessor::ApplySmearing(uint32_t inputTexture, uint32_t outputFBO) {
    // TODO: Implement motion blur/smearing effect
}

void EffectsProcessor::ApplyBlur(uint32_t inputTexture, uint32_t outputFBO) {
    // TODO: Implement Gaussian blur
}

void EffectsProcessor::ApplyBloom(uint32_t inputTexture, uint32_t outputFBO) {
    // TODO: Implement bloom effect (extract bright areas, blur, add back)
}

void EffectsProcessor::ApplyChromaticAberration(uint32_t inputTexture, uint32_t outputFBO) {
    // TODO: Implement chromatic aberration
}

void EffectsProcessor::ApplyVignette(uint32_t inputTexture, uint32_t outputFBO) {
    // TODO: Implement vignette effect
}

void EffectsProcessor::SetMaskTexture(uint32_t maskTexture) {
    maskTexture_ = maskTexture;
}

void EffectsProcessor::SetOutlineWidth(float width) {
    outlineWidth_ = width;
}

void EffectsProcessor::SetOutlineColor(float r, float g, float b, float a) {
    outlineColor_[0] = r;
    outlineColor_[1] = g;
    outlineColor_[2] = b;
    outlineColor_[3] = a;
}

void EffectsProcessor::SetShadowOffset(float x, float y) {
    shadowOffset_[0] = x;
    shadowOffset_[1] = y;
}

void EffectsProcessor::SetShadowBlur(float blur) {
    shadowBlur_ = blur;
}

void EffectsProcessor::SetSmearingAmount(float amount) {
    smearingAmount_ = amount;
}

void EffectsProcessor::SetBlurRadius(float radius) {
    blurRadius_ = radius;
}

void EffectsProcessor::SetBloomThreshold(float threshold) {
    bloomThreshold_ = threshold;
}

void EffectsProcessor::SetBloomIntensity(float intensity) {
    bloomIntensity_ = intensity;
}

void EffectsProcessor::SetVignetteStrength(float strength) {
    vignetteStrength_ = strength;
}

void EffectsProcessor::EnableEffect(EffectType effect, bool enabled) {
    int index = static_cast<int>(effect);
    if (index >= 0 && index < static_cast<int>(sizeof(effectsEnabled_) / sizeof(bool))) {
        effectsEnabled_[index] = enabled;
    }
}

bool EffectsProcessor::IsEffectEnabled(EffectType effect) const {
    int index = static_cast<int>(effect);
    if (index >= 0 && index < static_cast<int>(sizeof(effectsEnabled_) / sizeof(bool))) {
        return effectsEnabled_[index];
    }
    return false;
}

} // namespace Graphics
} // namespace AudioVisualizer
