# Audio Visualizer - Project Structure

This document provides an overview of the complete project structure.

## Directory Structure

```
AudioVisualizer/
│
├── CMakeLists.txt              # CMake build configuration
├── config.json                 # Default application configuration
├── LICENSE                     # MIT license
├── README.md                   # Main project documentation
├── CONTRIBUTING.md             # Contribution guidelines
├── .gitignore                  # Git ignore rules
├── setup.ps1                   # Windows setup script
│
├── include/                    # Public headers
│   └── AudioVisualizer.h       # Main header file
│
├── src/                        # Source code
│   ├── main.cpp                # Application entry point
│   │
│   ├── core/                   # Core functionality
│   │   ├── Application.h/cpp   # Main application controller
│   │   ├── AudioEngine.h/cpp   # Audio input and processing
│   │   ├── VisualizationEngine.h/cpp  # ProjectM integration
│   │   └── VideoRecorder.h/cpp # Video encoding with FFmpeg
│   │
│   ├── graphics/               # Graphics and rendering
│   │   ├── Renderer.h/cpp      # OpenGL rendering pipeline
│   │   ├── SpriteManager.h/cpp # Sprite/media handling
│   │   ├── EffectsProcessor.h/cpp  # Visual effects
│   │   └── ShaderManager.h/cpp # Shader management
│   │
│   ├── ui/                     # User interface
│   │   ├── MainWindow.h/cpp    # Main window and menu bar
│   │   ├── ControlPanel.h/cpp  # Control panel UI
│   │   └── PresetManager.h/cpp # Preset management UI
│   │
│   └── utils/                  # Utility classes
│       ├── Config.h/cpp        # Configuration management
│       ├── FileIO.h/cpp        # File I/O utilities
│       └── Logger.h/cpp        # Logging system
│
├── assets/                     # Application assets
│   ├── presets/                # ProjectM presets (.milk files)
│   ├── shaders/                # Custom GLSL shaders
│   └── fonts/                  # UI fonts
│
├── third_party/                # Third-party dependencies
│   ├── imgui/                  # Dear ImGui (download separately)
│   │   └── README.md
│   └── stb/                    # stb_image (download separately)
│       └── README.md
│
├── docs/                       # Documentation
│   ├── SETUP.md                # Setup guide
│   ├── BUILDING_WINDOWS.md     # Windows build instructions
│   └── EXAMPLES.md             # Example configurations
│
└── build/                      # Build output (generated)
    ├── bin/                    # Executables
    └── lib/                    # Libraries
```

## Component Overview

### Core Components

1. **Application** (`src/core/Application.cpp`)
   - Main application controller
   - Manages all subsystems
   - Handles main loop and lifecycle

2. **AudioEngine** (`src/core/AudioEngine.cpp`)
   - Audio input from microphone or line-in
   - Real-time audio processing
   - FFT analysis for frequency spectrum
   - Beat detection

3. **VisualizationEngine** (`src/core/VisualizationEngine.cpp`)
   - ProjectM integration
   - Preset management
   - Audio-reactive visualization

4. **VideoRecorder** (`src/core/VideoRecorder.cpp`)
   - FFmpeg integration
   - Real-time video encoding
   - Multiple codec support
   - Frame capture from OpenGL

### Graphics Components

1. **Renderer** (`src/graphics/Renderer.cpp`)
   - OpenGL rendering pipeline
   - Framebuffer management
   - Screen capture for recording

2. **SpriteManager** (`src/graphics/SpriteManager.cpp`)
   - Image/GIF/video sprite loading
   - Sprite animation
   - Texture management

3. **EffectsProcessor** (`src/graphics/EffectsProcessor.cpp`)
   - Post-processing effects
   - Masking, outline, shadow, smearing
   - Blur, bloom, vignette

4. **ShaderManager** (`src/graphics/ShaderManager.cpp`)
   - GLSL shader compilation
   - Shader program management
   - Uniform variable handling

### UI Components

1. **MainWindow** (`src/ui/MainWindow.cpp`)
   - Main menu bar
   - File operations
   - About dialog

2. **ControlPanel** (`src/ui/ControlPanel.cpp`)
   - Playback controls
   - Audio settings
   - Effect parameters
   - Sprite controls

3. **PresetManager** (`src/ui/PresetManager.cpp`)
   - Preset browser
   - Search functionality
   - Preset loading

### Utility Components

1. **Config** (`src/utils/Config.cpp`)
   - JSON-based configuration
   - Settings persistence
   - Nested key support

2. **FileIO** (`src/utils/FileIO.cpp`)
   - File operations
   - Directory listing
   - Path manipulation

3. **Logger** (`src/utils/Logger.cpp`)
   - Multi-level logging
   - File and console output
   - Thread-safe logging

## Key Technologies

- **C++17**: Modern C++ features
- **CMake**: Cross-platform build system
- **OpenGL 4.3**: Graphics rendering
- **GLFW**: Window and input handling
- **ProjectM**: Music visualization
- **FFmpeg**: Video encoding/decoding
- **PortAudio**: Audio input
- **Dear ImGui**: Immediate mode GUI
- **GLM**: Mathematics library
- **stb_image**: Image loading
- **nlohmann/json**: JSON parsing

## Build System

The project uses CMake for cross-platform builds:

- Automatically finds and links dependencies
- Supports Visual Studio, Make, Ninja
- Configurable build options
- Copies assets to build directory

## Configuration System

JSON-based configuration with hierarchical structure:

```json
{
  "audio": { ... },
  "visualization": { ... },
  "recording": { ... },
  "effects": { ... },
  "window": { ... }
}
```

Supports:
- Nested keys with dot notation
- Type-safe getters/setters
- Default values
- Runtime modification

## Architecture Patterns

1. **Component-Based**: Modular design with independent components
2. **Factory Pattern**: Object creation through managers
3. **Observer Pattern**: Event-driven updates
4. **Strategy Pattern**: Interchangeable algorithms (effects, codecs)
5. **RAII**: Resource management with smart pointers

## Thread Safety

- Logger: Thread-safe with mutex
- AudioEngine: Lock-free ring buffer for audio data
- Renderer: Single-threaded OpenGL context

## Performance Considerations

- OpenGL 4.3+ for modern rendering features
- FFT optimization with FFTW3 (optional)
- Frame buffer caching
- Efficient audio callback processing
- GPU-accelerated effects

## Future Enhancements

- [ ] Plugin system for custom effects
- [ ] Lua/Python scripting
- [ ] Network streaming
- [ ] VR/AR support
- [ ] Machine learning beat detection
- [ ] Multi-track audio support
- [ ] Advanced preset editor

## Getting Started

1. See `docs/SETUP.md` for installation
2. See `docs/BUILDING_WINDOWS.md` for Windows build
3. See `docs/EXAMPLES.md` for configuration examples
4. See `CONTRIBUTING.md` for development guidelines

## Support

- GitHub Issues: Bug reports and feature requests
- GitHub Discussions: Questions and community support
- Documentation: Comprehensive guides in `docs/`
