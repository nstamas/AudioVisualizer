# Audio Visualizer Desktop Application

A powerful audio visualization desktop application built with C++ and ProjectM, featuring psychedelic visuals, sprite injection, and video recording capabilities.

## Features

- 🎵 **Real-time Audio Visualization** - Complex geometric and psychedelic visuals powered by ProjectM
- 🎨 **Customizable Patterns** - Configure specific patterns, color gradients, and randomize settings
- 🎬 **Video Recording** - Save audio and visualizations to industry-standard video formats (MP4, AVI, MOV)
- 🖼️ **Media Injection** - Import images, GIFs, videos, and sprites into visualizations
- ✨ **Advanced Effects** - Masking, outlines, shadows, smearing, and more
- ⚙️ **User-Friendly GUI** - Intuitive interface for all controls and settings

## Architecture

```
AudioVisualizer/
├── src/
│   ├── main.cpp                    # Application entry point
│   ├── core/
│   │   ├── Application.cpp/h       # Main application controller
│   │   ├── AudioEngine.cpp/h       # Audio input and processing
│   │   ├── VisualizationEngine.cpp/h # ProjectM integration
│   │   └── VideoRecorder.cpp/h     # Video encoding and export
│   ├── graphics/
│   │   ├── Renderer.cpp/h          # OpenGL rendering pipeline
│   │   ├── SpriteManager.cpp/h     # Sprite/media handling
│   │   ├── EffectsProcessor.cpp/h  # Visual effects (masks, shadows, etc.)
│   │   └── ShaderManager.cpp/h     # Custom shader management
│   ├── ui/
│   │   ├── MainWindow.cpp/h        # Main application window
│   │   ├── ControlPanel.cpp/h      # User controls and settings
│   │   └── PresetManager.cpp/h     # Save/load preset configurations
│   └── utils/
│       ├── Config.cpp/h            # Configuration management
│       ├── FileIO.cpp/h            # File import/export utilities
│       └── Logger.cpp/h            # Logging system
├── include/                        # Public headers
├── assets/
│   ├── presets/                    # ProjectM presets
│   ├── shaders/                    # Custom GLSL shaders
│   └── fonts/                      # UI fonts
├── third_party/                    # External dependencies
├── build/                          # Build output
├── CMakeLists.txt                  # CMake build configuration
└── README.md
```

## Dependencies

### Required Libraries
- **ProjectM** (>= 4.0) - Music visualization engine
- **GLFW** (>= 3.3) - Window and input handling
- **OpenGL** (>= 4.3) - Graphics rendering
- **FFmpeg** (>= 5.0) - Video encoding/decoding
- **PortAudio** (>= 19.7) - Audio input
- **Dear ImGui** (>= 1.89) - User interface
- **GLM** (>= 0.9.9) - Mathematics library
- **stb_image** - Image loading
- **nlohmann/json** - Configuration serialization

### Optional Libraries
- **FFTW3** - Advanced FFT processing
- **OpenCV** - Advanced image/video processing

## Building

### Windows (Visual Studio)

```bash
# Install vcpkg (if not already installed)
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg && bootstrap-vcpkg.bat

# Install dependencies
vcpkg install glfw3:x64-windows portaudio:x64-windows glm:x64-windows nlohmann-json:x64-windows

# Build the project
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

### Linux

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install libglfw3-dev libportaudio2 libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libglm-dev

# Build the project
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### macOS

```bash
# Install dependencies via Homebrew
brew install glfw portaudio ffmpeg glm nlohmann-json

# Build the project
mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
```

## Usage

### Basic Usage

1. Launch the application
2. Select audio input source (microphone, line-in, or audio file)
3. Choose or customize visualization preset
4. Adjust parameters in real-time
5. Optionally inject sprites/media
6. Record output to video file

### Configuration

Edit `config.json` to set default parameters:

```json
{
  "audio": {
    "input_device": "default",
    "sample_rate": 44100,
    "buffer_size": 512
  },
  "visualization": {
    "width": 1920,
    "height": 1080,
    "fps": 60,
    "preset_path": "assets/presets/"
  },
  "recording": {
    "codec": "h264",
    "bitrate": "10M",
    "output_format": "mp4"
  }
}
```

### Keyboard Shortcuts

- `Space` - Play/Pause
- `R` - Start/Stop Recording
- `N` - Next preset
- `P` - Previous preset
- `S` - Save current configuration
- `L` - Load configuration
- `F11` - Toggle fullscreen
- `Esc` - Exit

## Development Roadmap

- [x] Project structure setup
- [ ] Core audio engine implementation
- [ ] ProjectM integration
- [ ] Basic OpenGL renderer
- [ ] ImGui interface
- [ ] FFmpeg video recorder
- [ ] Sprite injection system
- [ ] Effects processor (masking, shadows, etc.)
- [ ] Preset management system
- [ ] Configuration save/load
- [ ] Advanced audio analysis
- [ ] Custom shader support
- [ ] Performance optimization
- [ ] Cross-platform testing

## License

MIT License - See LICENSE file for details

## Contributing

Contributions are welcome! Please read CONTRIBUTING.md for guidelines.

## Credits

- **ProjectM** - https://github.com/projectM-visualizer/projectm
- **Dear ImGui** - https://github.com/ocornut/imgui
- **FFmpeg** - https://ffmpeg.org/
