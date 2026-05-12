/**
 * Audio Visualizer - Main Entry Point
 * 
 * A powerful desktop application for real-time audio visualization
 * with ProjectM integration, sprite injection, and video recording.
 */

#include "core/Application.h"
#include "utils/Logger.h"
#include "utils/Config.h"
#include <iostream>
#include <memory>
#include <exception>

int main(int argc, char* argv[]) {
    try {
        // Initialize logger
        AudioVisualizer::Utils::Logger::Initialize("audiovisualizer.log");
        AudioVisualizer::Utils::Logger::Info("Starting Audio Visualizer...");

        // Load configuration
        AudioVisualizer::Utils::Config config;
        if (argc > 1) {
            // Load custom config file if provided
            config.LoadFromFile(argv[1]);
        } else {
            // Load default config
            config.LoadFromFile("config.json");
        }

        // Create and run application
        auto app = std::make_unique<AudioVisualizer::Core::Application>(config);
        
        if (!app->Initialize()) {
            AudioVisualizer::Utils::Logger::Error("Failed to initialize application");
            return 1;
        }

        AudioVisualizer::Utils::Logger::Info("Application initialized successfully");
        
        // Main application loop
        int exitCode = app->Run();

        // Cleanup
        app->Shutdown();
        AudioVisualizer::Utils::Logger::Info("Application shutdown complete");

        return exitCode;

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        AudioVisualizer::Utils::Logger::Error("Fatal error: " + std::string(e.what()));
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred" << std::endl;
        AudioVisualizer::Utils::Logger::Error("Unknown fatal error occurred");
        return 1;
    }
}
