#ifndef AUDIOVISUALIZER_RENDERER_H
#define AUDIOVISUALIZER_RENDERER_H

#include <cstdint>

namespace AudioVisualizer {
#ifdef HAVE_FFMPEG
namespace Core {
    class VideoRecorder;
}
#endif

namespace Graphics {

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();

    bool Initialize();
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    void Composite();
    
    // Resize handling
    void Resize(int width, int height);
    
#ifdef HAVE_FFMPEG
    void CaptureFrame(Core::VideoRecorder* recorder);
#endif

    // Framebuffer management
    uint32_t GetMainFramebuffer() const { return mainFBO_; }
    uint32_t GetCompositeFramebuffer() const { return compositeFBO_; }

    // Getters
    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }

private:
    void CreateFramebuffers();
    void DestroyFramebuffers();

    int width_;
    int height_;

    // Framebuffers
    uint32_t mainFBO_;
    uint32_t mainTexture_;
    uint32_t mainDepthRBO_;

    uint32_t compositeFBO_;
    uint32_t compositeTexture_;

    // Capture buffer
    uint8_t* captureBuffer_;

    bool isInitialized_;
};

} // namespace Graphics
} // namespace AudioVisualizer

#endif // AUDIOVISUALIZER_RENDERER_H
