# Quick Start Script for Windows
# This script helps set up the development environment

Write-Host "Audio Visualizer - Setup Script" -ForegroundColor Cyan
Write-Host "================================`n" -ForegroundColor Cyan

# Check if vcpkg is installed
$vcpkgPath = "C:\vcpkg"
if (-not (Test-Path $vcpkgPath)) {
    Write-Host "vcpkg not found. Installing..." -ForegroundColor Yellow
    
    git clone https://github.com/microsoft/vcpkg.git $vcpkgPath
    Set-Location $vcpkgPath
    .\bootstrap-vcpkg.bat
    .\vcpkg integrate install
    
    Write-Host "vcpkg installed successfully!`n" -ForegroundColor Green
} else {
    Write-Host "vcpkg already installed`n" -ForegroundColor Green
}

# Install dependencies
Write-Host "Installing dependencies..." -ForegroundColor Yellow

$packages = @(
    "glfw3:x64-windows",
    "portaudio:x64-windows",
    "glm:x64-windows",
    "nlohmann-json:x64-windows",
    "ffmpeg:x64-windows"
)

foreach ($package in $packages) {
    Write-Host "  Installing $package..." -ForegroundColor Gray
    & "$vcpkgPath\vcpkg.exe" install $package
}

Write-Host "`nDependencies installed!`n" -ForegroundColor Green

# Download third-party libraries
Write-Host "Downloading third-party libraries..." -ForegroundColor Yellow

$imguiUrl = "https://github.com/ocornut/imgui/archive/refs/heads/master.zip"
$stbUrl = "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h"

# Create directories
$thirdPartyDir = "third_party"
New-Item -ItemType Directory -Force -Path "$thirdPartyDir\imgui" | Out-Null
New-Item -ItemType Directory -Force -Path "$thirdPartyDir\stb" | Out-Null

# Download stb_image.h
Write-Host "  Downloading stb_image.h..." -ForegroundColor Gray
Invoke-WebRequest -Uri $stbUrl -OutFile "$thirdPartyDir\stb\stb_image.h"

Write-Host "  Note: Please manually download Dear ImGui from:" -ForegroundColor Yellow
Write-Host "        https://github.com/ocornut/imgui/archive/refs/heads/master.zip" -ForegroundColor Yellow
Write-Host "        Extract to third_party/imgui/" -ForegroundColor Yellow

Write-Host "`nSetup complete!`n" -ForegroundColor Green

# Build the project
$build = Read-Host "Do you want to build the project now? (y/n)"

if ($build -eq "y" -or $build -eq "Y") {
    Write-Host "`nBuilding project..." -ForegroundColor Yellow
    
    New-Item -ItemType Directory -Force -Path "build" | Out-Null
    Set-Location build
    
    cmake .. -DCMAKE_TOOLCHAIN_FILE="$vcpkgPath\scripts\buildsystems\vcpkg.cmake"
    cmake --build . --config Release
    
    Write-Host "`nBuild complete!" -ForegroundColor Green
    Write-Host "Executable location: build\bin\Release\AudioVisualizer.exe" -ForegroundColor Cyan
    
    # Copy config and assets
    Copy-Item "..\config.json" "bin\Release\" -Force
    Copy-Item "..\assets" "bin\Release\" -Recurse -Force
    
    Write-Host "`nYou can now run the application!" -ForegroundColor Green
}

Write-Host "`nFor more information, see docs/SETUP.md" -ForegroundColor Cyan
