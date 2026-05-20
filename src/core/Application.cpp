#include "Application.h"
#include "AudioEngine.h"
#include "VisualizationEngine.h"
#ifdef HAVE_FFMPEG
#include "VideoRecorder.h"
#endif
#include "graphics/Renderer.h"
#include "graphics/SpriteManager.h"
#include "graphics/EffectsProcessor.h"
#include "graphics/ShaderManager.h"
#include "ui/MainWindow.h"
#include "ui/ControlPanel.h"
#include "ui/PresetManager.h"
#include "ui/LogWindow.h"
#include "utils/Logger.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <stb_image.h>

namespace AudioVisualizer {
namespace Core {

// GLFW Callbacks
static void ErrorCallback(int error, const char* description) {
    Utils::Logger::Error("GLFW Error (" + std::to_string(error) + "): " + description);
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
            case GLFW_KEY_SPACE:
                if (app->IsPlaying()) {
                    app->Pause();
                } else {
                    app->Play();
                }
                break;
            case GLFW_KEY_R:
                if (app->IsRecording()) {
                    app->StopRecording();
                } else {
                    app->StartRecording("output.mp4");
                }
                break;
            case GLFW_KEY_F11:
                // Toggle fullscreen
                break;
        }
    }
}

static void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    // The actual resize will be handled in the main loop to avoid threading issues
}

Application::Application(const Utils::Config& config)
    : config_(config)
    , window_(nullptr)
    , windowWidth_(1920)
    , windowHeight_(1080)
    , isFullscreen_(false)
    , isRunning_(false)
    , isPlaying_(false)
    , isRecording_(false)
    , lastFrameTime_(0.0)
    , fps_(0.0f)
{
}

Application::~Application() {
    Shutdown();
}

bool Application::Initialize() {
    Utils::Logger::Info("Initializing application...");

    // Initialize GLFW
    glfwSetErrorCallback(ErrorCallback);
    
    if (!glfwInit()) {
        Utils::Logger::Error("Failed to initialize GLFW");
        return false;
    }

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x MSAA

    // Get window dimensions from config
    windowWidth_ = config_.GetInt("visualization.width", 1920);
    windowHeight_ = config_.GetInt("visualization.height", 1080);

    // Create window
    window_ = glfwCreateWindow(windowWidth_, windowHeight_, 
                                "Audio Visualizer", nullptr, nullptr);
    if (!window_) {
        Utils::Logger::Error("Failed to create GLFW window");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window_);
    glfwSetWindowUserPointer(window_, this);
    glfwSetKeyCallback(window_, KeyCallback);
    glfwSetFramebufferSizeCallback(window_, FramebufferSizeCallback);
    glfwSwapInterval(1); // Enable vsync

    // Set window icon - try multiple paths
    GLFWimage icon;
    int channels;
    const char* iconPaths[] = {
        "icon.png",
        "bin/Release/icon.png",
        "../icon.png"
    };
    
    bool iconLoaded = false;
    for (const char* path : iconPaths) {
        icon.pixels = stbi_load(path, &icon.width, &icon.height, &channels, 4);
        if (icon.pixels) {
            glfwSetWindowIcon(window_, 1, &icon);
            stbi_image_free(icon.pixels);
            Utils::Logger::Info(std::string("Window icon loaded from: ") + path);
            iconLoaded = true;
            break;
        }
    }
    
    if (!iconLoaded) {
        Utils::Logger::Warning("Failed to load window icon from any path");
    }

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        Utils::Logger::Error("Failed to initialize GLEW: " + 
            std::string(reinterpret_cast<const char*>(glewGetErrorString(glewError))));
        glfwTerminate();
        return false;
    }

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    // Increase font/UI scale by 25%
    io.FontGlobalScale = 1.25f;

    ImGui::StyleColorsDark();
    
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 430");

    // Initialize core components
    Utils::Logger::Info("Initializing audio engine...");
    audioEngine_ = std::make_unique<AudioEngine>(config_);
    if (!audioEngine_->Initialize()) {
        Utils::Logger::Error("Failed to initialize audio engine");
        return false;
    }

    Utils::Logger::Info("Initializing visualization engine...");
    visualizationEngine_ = std::make_unique<VisualizationEngine>(config_);
    if (!visualizationEngine_->Initialize()) {
        Utils::Logger::Error("Failed to initialize visualization engine");
        return false;
    }

    Utils::Logger::Info("Initializing renderer...");
    renderer_ = std::make_unique<Graphics::Renderer>(windowWidth_, windowHeight_);
    if (!renderer_->Initialize()) {
        Utils::Logger::Error("Failed to initialize renderer");
        return false;
    }

    Utils::Logger::Info("Initializing sprite manager...");
    spriteManager_ = std::make_unique<Graphics::SpriteManager>();
    if (!spriteManager_->Initialize()) {
        Utils::Logger::Error("Failed to initialize sprite manager");
        return false;
    }
    
    Utils::Logger::Info("Initializing shader manager...");
    shaderManager_ = std::make_unique<Graphics::ShaderManager>();
    if (!shaderManager_->Initialize()) {
        Utils::Logger::Error("Failed to initialize shader manager");
        return false;
    }
    
    // Initialize shader-based visualizations
    if (!visualizationEngine_->InitializeShaders(shaderManager_.get())) {
        Utils::Logger::Warning("Failed to initialize shader visualizations");
    }

    Utils::Logger::Info("Initializing effects processor...");
    effectsProcessor_ = std::make_unique<Graphics::EffectsProcessor>();
    if (!effectsProcessor_->Initialize()) {
        Utils::Logger::Error("Failed to initialize effects processor");
        return false;
    }

#ifdef HAVE_FFMPEG
    Utils::Logger::Info("Initializing video recorder...");
    videoRecorder_ = std::make_unique<VideoRecorder>(config_);
    if (!videoRecorder_->Initialize(windowWidth_, windowHeight_)) {
        Utils::Logger::Error("Failed to initialize video recorder");
        return false;
    }
#else
    Utils::Logger::Warning("Video recorder disabled (FFmpeg not available)");
#endif

    // Initialize UI components
    mainWindow_ = std::make_unique<UI::MainWindow>(this);
    controlPanel_ = std::make_unique<UI::ControlPanel>(this);
    presetManager_ = std::make_unique<UI::PresetManager>(this);
    logWindow_ = std::make_unique<UI::LogWindow>();

    isRunning_ = true;
    Utils::Logger::Info("Application initialized successfully");
    return true;
}

int Application::Run() {
    Utils::Logger::Info("Starting main loop...");
    lastFrameTime_ = glfwGetTime();

    while (isRunning_ && !glfwWindowShouldClose(window_)) {
        // Check for window resize
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window_, &fbWidth, &fbHeight);
        if (fbWidth != windowWidth_ || fbHeight != windowHeight_) {
            windowWidth_ = fbWidth;
            windowHeight_ = fbHeight;
            renderer_->Resize(fbWidth, fbHeight);
            visualizationEngine_->Resize(fbWidth, fbHeight);
        }
        
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastFrameTime_);
        lastFrameTime_ = currentTime;
        fps_ = 1.0f / deltaTime;

        ProcessInput();
        Update(deltaTime);
        Render();

        glfwSwapBuffers(window_);
        glfwPollEvents();
    }
    
    return 0;
}

void Application::Shutdown() {
    if (!isRunning_ && window_ == nullptr) {
        return; // Already shut down
    }

    Utils::Logger::Info("Shutting down application...");

    if (isRecording_) {
        StopRecording();
    }

    if (isPlaying_) {
        Stop();
    }

    // Cleanup UI
    presetManager_.reset();
    controlPanel_.reset();
    mainWindow_.reset();

#ifdef HAVE_FFMPEG
    videoRecorder_.reset();
#endif
    effectsProcessor_.reset();
    spriteManager_.reset();
    renderer_.reset();
    visualizationEngine_.reset();
    audioEngine_.reset();

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Cleanup GLFW
    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
    glfwTerminate();

    isRunning_ = false;
}

void Application::ProcessInput() {
    // Additional input processing if needed
}

void Application::Update(float deltaTime) {
    if (isPlaying_) {
        // Update audio engine
        audioEngine_->Update();

        // Get audio data and pass to visualization
        auto audioData = audioEngine_->GetAudioData();
        visualizationEngine_->Update(audioData, deltaTime);

        // Update sprites
        spriteManager_->Update(deltaTime);
    }
}

void Application::Render() {
    // Begin frame (render visualization to offscreen buffer)
    renderer_->BeginFrame();

    if (isPlaying_) {
        // Render visualization
        visualizationEngine_->Render();

        // Render sprites with effects
        spriteManager_->Render();
        effectsProcessor_->Apply();
#ifdef HAVE_FFMPEG
        if (isRecording_) {
            renderer_->CaptureFrame(videoRecorder_.get());
        }
#endif
    }

    // Composite final image (always, even if not playing - will be black screen)
    renderer_->Composite();
    
    // Present composite to screen
    renderer_->EndFrame();

    // Render UI on top of everything (directly to screen)
    RenderUI();
}

void Application::RenderUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Render UI components
    mainWindow_->Render();
    controlPanel_->Render();
    presetManager_->Render();
    logWindow_->Render();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Application::Play() {
    if (!isPlaying_) {
        Utils::Logger::Info("Starting playback");
        audioEngine_->Start();
        isPlaying_ = true;
    }
}

void Application::Pause() {
    if (isPlaying_) {
        Utils::Logger::Info("Pausing playback");
        audioEngine_->Pause();
        isPlaying_ = false;
    }
}

void Application::Stop() {
    if (isPlaying_) {
        Utils::Logger::Info("Stopping playback");
        audioEngine_->Stop();
        isPlaying_ = false;
    }
}

bool Application::LoadAudioFile(const std::string& filePath) {
    Utils::Logger::Info("Loading audio file: " + filePath);
    return audioEngine_->LoadAudioFile(filePath);
}

void Application::StartRecording(const std::string& outputPath) {
#ifdef HAVE_FFMPEG
    if (!isRecording_) {
        Utils::Logger::Info("Starting recording: " + outputPath);
        if (videoRecorder_->StartRecording(outputPath)) {
            isRecording_ = true;
        } else {
            Utils::Logger::Error("Failed to start recording");
        }
    }
#else
    Utils::Logger::Warning("Recording not available - FFmpeg not compiled");
#endif
}

void Application::StopRecording() {
#ifdef HAVE_FFMPEG
    if (isRecording_) {
        Utils::Logger::Info("Stopping recording");
        videoRecorder_->StopRecording();
        isRecording_ = false;
    }
#endif
}

} // namespace Core
} // namespace AudioVisualizer
