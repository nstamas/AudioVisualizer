#ifndef AUDIOVISUALIZER_PRESETMANAGER_H
#define AUDIOVISUALIZER_PRESETMANAGER_H

#include <string>
#include <vector>

namespace AudioVisualizer {
namespace Core {
    class Application;
}

namespace UI {

class PresetManager {
public:
    explicit PresetManager(Core::Application* app);
    ~PresetManager();

    void Render();

private:
    void RefreshPresetList();
    void LoadPreset(int index);

    Core::Application* app_;
    bool isVisible_;
    std::vector<std::string> presetNames_;
    int selectedPreset_;
    char searchBuffer_[256];
};

} // namespace UI
} // namespace AudioVisualizer

#endif // AUDIOVISUALIZER_PRESETMANAGER_H
