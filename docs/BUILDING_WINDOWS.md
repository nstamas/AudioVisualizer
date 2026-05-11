# Building on Windows

## Prerequisites

1. **Install Visual Studio 2019 or later**
   - Ensure "Desktop development with C++" workload is installed
   - Include CMake tools for Windows

2. **Install vcpkg** (Windows package manager for C++)
   ```powershell
   git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
   cd C:\vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg integrate install
   ```

3. **Install dependencies using vcpkg**
   ```powershell
   .\vcpkg install glfw3:x64-windows
   .\vcpkg install portaudio:x64-windows
   .\vcpkg install glm:x64-windows
   .\vcpkg install nlohmann-json:x64-windows
   .\vcpkg install ffmpeg:x64-windows
   ```

## Building the Project

### Using Visual Studio

1. Open Visual Studio
2. Select "Open a local folder" and choose the AudioVisualizer directory
3. Visual Studio will automatically detect CMakeLists.txt
4. Select Build > Build All

### Using Command Line

```powershell
# Configure
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake

# Build
cmake --build . --config Release

# The executable will be in build\bin\Release\AudioVisualizer.exe
```

## Installing ProjectM

ProjectM may need to be installed manually:

```powershell
# Clone ProjectM
git clone https://github.com/projectM-visualizer/projectm.git
cd projectm

# Build and install
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build . --config Release
cmake --install .
```

## Running the Application

1. Copy `config.json` to the build output directory
2. Copy the `assets` folder to the build output directory
3. Run `AudioVisualizer.exe`

## Troubleshooting

### Missing DLLs

If you get DLL errors, ensure that:
- The vcpkg installed DLLs are in your PATH or the executable directory
- FFmpeg DLLs are accessible
- PortAudio DLL is in the executable directory

### OpenGL Errors

- Update your graphics drivers
- Ensure your GPU supports OpenGL 4.3 or later

### Audio Input Issues

- Check that PortAudio can access your audio devices
- Run the application as Administrator if needed
- Check Windows audio permissions

## Performance Tips

- Disable Windows Game Bar and DVR if experiencing frame drops
- Close other audio applications to reduce conflicts
- Use Release build for best performance
- Reduce resolution or FPS in config.json if needed
