#ifndef AUDIOVISUALIZER_MAINWINDOW_H
#define AUDIOVISUALIZER_MAINWINDOW_H

namespace AudioVisualizer {
namespace Core {
    class Application;
}

namespace UI {

class MainWindow {
public:
    explicit MainWindow(Core::Application* app);
    ~MainWindow();

    void Render();

private:
    Core::Application* app_;
    bool showAbout_;
    bool showSettings_;
};

} // namespace UI
} // namespace AudioVisualizer

#endif // AUDIOVISUALIZER_MAINWINDOW_H
