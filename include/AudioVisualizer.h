#ifndef AUDIOVISUALIZER_H
#define AUDIOVISUALIZER_H

/**
 * Audio Visualizer
 * 
 * A powerful desktop application for real-time audio visualization
 * with ProjectM integration, sprite injection, and video recording.
 * 
 * Main Components:
 * - Core: Application, AudioEngine, VisualizationEngine, VideoRecorder
 * - Graphics: Renderer, SpriteManager, EffectsProcessor, ShaderManager
 * - UI: MainWindow, ControlPanel, PresetManager
 * - Utils: Config, FileIO, Logger
 */

// Version information
#define AUDIOVISUALIZER_VERSION_MAJOR 1
#define AUDIOVISUALIZER_VERSION_MINOR 0
#define AUDIOVISUALIZER_VERSION_PATCH 0

// Core components
#include "core/Application.h"
#include "core/AudioEngine.h"
#include "core/VisualizationEngine.h"
#include "core/VideoRecorder.h"

// Graphics components
#include "graphics/Renderer.h"
#include "graphics/SpriteManager.h"
#include "graphics/EffectsProcessor.h"
#include "graphics/ShaderManager.h"

// UI components
#include "ui/MainWindow.h"
#include "ui/ControlPanel.h"
#include "ui/PresetManager.h"

// Utility components
#include "utils/Config.h"
#include "utils/FileIO.h"
#include "utils/Logger.h"

namespace AudioVisualizer {

// Utility functions
inline const char* GetVersionString() {
    static char version[32];
    snprintf(version, sizeof(version), "%d.%d.%d", 
             AUDIOVISUALIZER_VERSION_MAJOR,
             AUDIOVISUALIZER_VERSION_MINOR,
             AUDIOVISUALIZER_VERSION_PATCH);
    return version;
}

} // namespace AudioVisualizer

#endif // AUDIOVISUALIZER_H
