#ifndef AUDIOVISUALIZER_CONTROLPANEL_H
#define AUDIOVISUALIZER_CONTROLPANEL_H

namespace AudioVisualizer {
namespace Core {
    class Application;
}

namespace UI {

class ControlPanel {
public:
    explicit ControlPanel(Core::Application* app);
    ~ControlPanel();

    void Render();

private:
    void RenderAudioControls();
    void RenderVisualizationControls();
    void RenderEffectsControls();
    void RenderSpriteControls();

    Core::Application* app_;
    bool isVisible_;
    
    // Effect parameters
    bool enableMasking_;
    bool enableOutline_;
    bool enableShadow_;
    bool enableSmearing_;
    bool enableBlur_;
    bool enableBloom_;
    
    float outlineWidth_;
    float shadowOffset_[2];
    float smearingAmount_;
    float blurRadius_;
    float bloomThreshold_;
};

} // namespace UI
} // namespace AudioVisualizer

#endif // AUDIOVISUALIZER_CONTROLPANEL_H
