#ifndef AUDIOVISUALIZER_EFFECTSPROCESSOR_H
#define AUDIOVISUALIZER_EFFECTSPROCESSOR_H

#include <string>
#include <cstdint>

namespace AudioVisualizer {
namespace Graphics {

enum class EffectType {
    None,
    Masking,
    Outline,
    Shadow,
    Smearing,
    Blur,
    Bloom,
    ChromaticAberration,
    Vignette
};

class EffectsProcessor {
public:
    EffectsProcessor();
    ~EffectsProcessor();

    bool Initialize();
    void Shutdown();

    void Apply();
    void ApplyEffect(EffectType effect, uint32_t inputTexture, uint32_t outputFBO);

    // Effect parameters
    void SetMaskTexture(uint32_t maskTexture);
    void SetOutlineWidth(float width);
    void SetOutlineColor(float r, float g, float b, float a);
    void SetShadowOffset(float x, float y);
    void SetShadowBlur(float blur);
    void SetSmearingAmount(float amount);
    void SetBlurRadius(float radius);
    void SetBloomThreshold(float threshold);
    void SetBloomIntensity(float intensity);
    void SetVignetteStrength(float strength);

    // Effect toggles
    void EnableEffect(EffectType effect, bool enabled);
    bool IsEffectEnabled(EffectType effect) const;

private:
    void InitializeShaders();
    void ApplyMasking(uint32_t inputTexture, uint32_t outputFBO);
    void ApplyOutline(uint32_t inputTexture, uint32_t outputFBO);
    void ApplyShadow(uint32_t inputTexture, uint32_t outputFBO);
    void ApplySmearing(uint32_t inputTexture, uint32_t outputFBO);
    void ApplyBlur(uint32_t inputTexture, uint32_t outputFBO);
    void ApplyBloom(uint32_t inputTexture, uint32_t outputFBO);
    void ApplyChromaticAberration(uint32_t inputTexture, uint32_t outputFBO);
    void ApplyVignette(uint32_t inputTexture, uint32_t outputFBO);

    // Shader programs
    uint32_t maskingShader_;
    uint32_t outlineShader_;
    uint32_t shadowShader_;
    uint32_t smearingShader_;
    uint32_t blurShader_;
    uint32_t bloomShader_;
    uint32_t chromaticShader_;
    uint32_t vignetteShader_;

    // Effect parameters
    uint32_t maskTexture_;
    float outlineWidth_;
    float outlineColor_[4];
    float shadowOffset_[2];
    float shadowBlur_;
    float smearingAmount_;
    float blurRadius_;
    float bloomThreshold_;
    float bloomIntensity_;
    float vignetteStrength_;

    // Effect states
    bool effectsEnabled_[static_cast<int>(EffectType::Vignette) + 1];

    // Render quad
    uint32_t quadVAO_;
    uint32_t quadVBO_;

    bool isInitialized_;
};

} // namespace Graphics
} // namespace AudioVisualizer

#endif // AUDIOVISUALIZER_EFFECTSPROCESSOR_H
