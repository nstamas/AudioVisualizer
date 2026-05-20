#include "Renderer.h"
#include "core/VideoRecorder.h"
#include "utils/Logger.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstring>

namespace AudioVisualizer {
namespace Graphics {

Renderer::Renderer(int width, int height)
    : width_(width)
    , height_(height)
    , mainFBO_(0)
    , mainTexture_(0)
    , mainDepthRBO_(0)
    , compositeFBO_(0)
    , compositeTexture_(0)
    , captureBuffer_(nullptr)
    , isInitialized_(false)
{
}

Renderer::~Renderer() {
    Shutdown();
}

bool Renderer::Initialize() {
    Utils::Logger::Info("Initializing renderer...");

    // Initialize OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    // Create framebuffers
    CreateFramebuffers();

    // Allocate capture buffer
    captureBuffer_ = new uint8_t[width_ * height_ * 4];

    isInitialized_ = true;
    Utils::Logger::Info("Renderer initialized");
    return true;
}

void Renderer::Shutdown() {
    if (!isInitialized_) {
        return;
    }

    DestroyFramebuffers();

    if (captureBuffer_) {
        delete[] captureBuffer_;
        captureBuffer_ = nullptr;
    }

    isInitialized_ = false;
}

void Renderer::CreateFramebuffers() {
    // Main framebuffer (for visualization rendering)
    glGenFramebuffers(1, &mainFBO_);
    glBindFramebuffer(GL_FRAMEBUFFER, mainFBO_);

    // Create texture
    glGenTextures(1, &mainTexture_);
    glBindTexture(GL_TEXTURE_2D, mainTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, 
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                          GL_TEXTURE_2D, mainTexture_, 0);

    // Create depth renderbuffer
    glGenRenderbuffers(1, &mainDepthRBO_);
    glBindRenderbuffer(GL_RENDERBUFFER, mainDepthRBO_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width_, height_);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 
                             GL_RENDERBUFFER, mainDepthRBO_);

    // Check framebuffer status
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        Utils::Logger::Error("Main framebuffer is not complete");
    }

    // Composite framebuffer (for final composition)
    glGenFramebuffers(1, &compositeFBO_);
    glBindFramebuffer(GL_FRAMEBUFFER, compositeFBO_);

    glGenTextures(1, &compositeTexture_);
    glBindTexture(GL_TEXTURE_2D, compositeTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, 
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                          GL_TEXTURE_2D, compositeTexture_, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        Utils::Logger::Error("Composite framebuffer is not complete");
    }

    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DestroyFramebuffers() {
    if (mainFBO_) {
        glDeleteFramebuffers(1, &mainFBO_);
        mainFBO_ = 0;
    }

    if (mainTexture_) {
        glDeleteTextures(1, &mainTexture_);
        mainTexture_ = 0;
    }

    if (mainDepthRBO_) {
        glDeleteRenderbuffers(1, &mainDepthRBO_);
        mainDepthRBO_ = 0;
    }

    if (compositeFBO_) {
        glDeleteFramebuffers(1, &compositeFBO_);
        compositeFBO_ = 0;
    }

    if (compositeTexture_) {
        glDeleteTextures(1, &compositeTexture_);
        compositeTexture_ = 0;
    }
}

void Renderer::Resize(int width, int height) {
    if (width == width_ && height == height_) {
        return; // No change
    }
    
    width_ = width;
    height_ = height;
    
    // Recreate framebuffers with new size
    DestroyFramebuffers();
    CreateFramebuffers();
}

void Renderer::BeginFrame() {
    // Bind main framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, mainFBO_);
    glViewport(0, 0, width_, height_);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::EndFrame() {
    // Render composite framebuffer to screen
    glBindFramebuffer(GL_READ_FRAMEBUFFER, compositeFBO_);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glViewport(0, 0, width_, height_);
    glBlitFramebuffer(0, 0, width_, height_, 0, 0, width_, height_,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    
    // Unbind framebuffer (now rendering to screen for UI)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::Composite() {
    // Composite all layers to final framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, compositeFBO_);
    glViewport(0, 0, width_, height_);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Copy main texture to composite using blit
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mainFBO_);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, compositeFBO_);
    glBlitFramebuffer(0, 0, width_, height_, 0, 0, width_, height_,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

#ifdef HAVE_FFMPEG
void Renderer::CaptureFrame(Core::VideoRecorder* recorder) {
    if (!recorder) {
        return;
    }

    // Read pixels from composite framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, compositeFBO_);
    glReadPixels(0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, captureBuffer_);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Flip image vertically (OpenGL texture coordinates are bottom-up)
    int rowSize = width_ * 4;
    uint8_t* tempRow = new uint8_t[rowSize];
    
    for (int y = 0; y < height_ / 2; ++y) {
        int topOffset = y * rowSize;
        int bottomOffset = (height_ - 1 - y) * rowSize;
        
        std::memcpy(tempRow, captureBuffer_ + topOffset, rowSize);
        std::memcpy(captureBuffer_ + topOffset, captureBuffer_ + bottomOffset, rowSize);
        std::memcpy(captureBuffer_ + bottomOffset, tempRow, rowSize);
    }
    
    delete[] tempRow;

    // Write frame to recorder
    recorder->WriteFrame(captureBuffer_);
}
#endif

} // namespace Graphics
} // namespace AudioVisualizer
