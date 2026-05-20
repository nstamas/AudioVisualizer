#ifndef AUDIOVISUALIZER_LOGWINDOW_H
#define AUDIOVISUALIZER_LOGWINDOW_H

namespace AudioVisualizer {
namespace UI {

class LogWindow {
public:
    LogWindow();
    ~LogWindow();

    void Render();
    
    bool IsVisible() const { return isVisible_; }
    void SetVisible(bool visible) { isVisible_ = visible; }

private:
    bool isVisible_;
    bool autoScroll_;
    int levelFilter_; // 0=All, 1=Info+, 2=Warn+, 3=Error only
};

} // namespace UI
} // namespace AudioVisualizer

#endif // AUDIOVISUALIZER_LOGWINDOW_H
