# Setup Guide

## Quick Start

### 1. Install Prerequisites

#### Windows
- Visual Studio 2019+ with C++ development tools
- CMake 3.20+
- vcpkg (package manager)

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install build-essential cmake git
sudo apt-get install libglfw3-dev libportaudio2 libavcodec-dev libavformat-dev
sudo apt-get install libavutil-dev libswscale-dev libglm-dev
```

#### macOS
```bash
brew install cmake glfw portaudio ffmpeg glm nlohmann-json
```

### 2. Download Third-Party Libraries

Some libraries need to be manually downloaded:

1. **Dear ImGui** (https://github.com/ocornut/imgui)
   - Download the latest release
   - Extract to `third_party/imgui/`

2. **stb_image** (https://github.com/nothings/stb)
   - Download `stb_image.h`
   - Place in `third_party/stb/`

3. **ProjectM** (optional, for full visualization support)
   - Build from source: https://github.com/projectM-visualizer/projectm
   - Or install via package manager

### 3. Build the Project

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### 4. First Run

1. Copy `config.json` to the build output directory
2. Run the executable:
   - Windows: `bin\Release\AudioVisualizer.exe`
   - Linux/macOS: `bin/AudioVisualizer`

## Configuration

Edit `config.json` to customize:

### Audio Settings
- `input_device`: Audio input source (default, microphone, line-in)
- `sample_rate`: Audio sample rate (44100, 48000, etc.)
- `buffer_size`: Audio buffer size (256, 512, 1024)
- `fft_size`: FFT size for frequency analysis (1024, 2048, 4096)

### Visualization Settings
- `width`, `height`: Render resolution
- `fps`: Target frame rate
- `preset_path`: Path to ProjectM presets
- `preset_duration`: Seconds per preset
- `beat_sensitivity`: How reactive to beats (0.0 - 2.0)

### Recording Settings
- `codec`: Video codec (libx264, libx265, etc.)
- `bitrate`: Video bitrate (5M, 10M, 20M)
- `output_format`: Video format (mp4, avi, mkv)

## Adding Presets

ProjectM presets (.milk files) can be placed in `assets/presets/`.
Download preset packs from the ProjectM website or community sources.

## Custom Shaders

Custom GLSL shaders can be added to `assets/shaders/`:
- `vertex.glsl`: Vertex shader
- `fragment.glsl`: Fragment shader

## Keyboard Shortcuts

- `Space`: Play/Pause
- `R`: Start/Stop Recording
- `N`: Next preset
- `P`: Previous preset
- `S`: Save current configuration
- `L`: Load configuration
- `F11`: Toggle fullscreen
- `Esc`: Exit

## Performance Optimization

### High-End Systems
```json
{
  "visualization": {
    "width": 3840,
    "height": 2160,
    "fps": 60
  },
  "recording": {
    "bitrate": "20M"
  }
}
```

### Mid-Range Systems
```json
{
  "visualization": {
    "width": 1920,
    "height": 1080,
    "fps": 60
  },
  "recording": {
    "bitrate": "10M"
  }
}
```

### Low-End Systems
```json
{
  "visualization": {
    "width": 1280,
    "height": 720,
    "fps": 30
  },
  "recording": {
    "bitrate": "5M"
  }
}
```

## Troubleshooting

### No Audio Input
- Check audio device permissions
- Try different input device in config
- Ensure PortAudio is correctly installed

### Low Frame Rate
- Reduce resolution in config
- Lower FPS target
- Disable effects in control panel
- Close other GPU-intensive applications

### Recording Issues
- Ensure FFmpeg is properly installed
- Check disk space
- Verify write permissions for output directory
- Try different codec (h264 vs h265)

### Visualization Not Showing
- Ensure ProjectM is installed
- Check preset_path in config
- Verify presets exist in assets/presets/

## Advanced Usage

### Command Line Arguments

```bash
# Use custom config file
AudioVisualizer custom_config.json

# Examples of future CLI arguments (to be implemented)
AudioVisualizer --input "audio_file.mp3" --output "visualization.mp4"
AudioVisualizer --preset "Cosmic Waves" --duration 120
```

### Scripting

The application can be extended with custom scripts:
- Preset sequencing
- Automated recording sessions
- Batch processing of audio files

## Getting Help

- Check the documentation in `docs/`
- Review example configurations in `examples/`
- Submit issues on GitHub
- Join the community Discord
