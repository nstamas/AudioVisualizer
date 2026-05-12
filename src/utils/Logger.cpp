#include "Logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace AudioVisualizer {
namespace Utils {

// Static member initialization
std::ofstream Logger::logFile_;
std::recursive_mutex Logger::logMutex_;
LogLevel Logger::currentLevel_ = LogLevel::Info;
bool Logger::consoleOutputEnabled_ = true;
bool Logger::initialized_ = false;

void Logger::Initialize(const std::string& logFilePath) {
    std::lock_guard<std::recursive_mutex> lock(logMutex_);

    if (initialized_) {
        return;
    }

    logFile_.open(logFilePath, std::ios::out | std::ios::app);
    if (logFile_.is_open()) {
        initialized_ = true;
        Log(LogLevel::Info, "Logger initialized");
    } else {
        std::cerr << "Failed to open log file: " << logFilePath << std::endl;
    }
}

void Logger::Shutdown() {
    std::lock_guard<std::recursive_mutex> lock(logMutex_);

    if (!initialized_) {
        return;
    }

    Log(LogLevel::Info, "Logger shutdown");
    
    if (logFile_.is_open()) {
        logFile_.close();
    }

    initialized_ = false;
}

void Logger::Debug(const std::string& message) {
    Log(LogLevel::Debug, message);
}

void Logger::Info(const std::string& message) {
    Log(LogLevel::Info, message);
}

void Logger::Warning(const std::string& message) {
    Log(LogLevel::Warning, message);
}

void Logger::Error(const std::string& message) {
    Log(LogLevel::Error, message);
}

void Logger::Log(LogLevel level, const std::string& message) {
    if (level < currentLevel_) {
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(logMutex_);

    std::string timestamp = GetTimestamp();
    std::string levelStr = GetLevelString(level);
    std::string logMessage = "[" + timestamp + "] [" + levelStr + "] " + message;

    // Output to console if enabled
    if (consoleOutputEnabled_) {
        if (level == LogLevel::Error) {
            std::cerr << logMessage << std::endl;
        } else {
            std::cout << logMessage << std::endl;
        }
    }

    // Output to file
    if (initialized_ && logFile_.is_open()) {
        logFile_ << logMessage << std::endl;
        logFile_.flush();
    }
}

std::string Logger::GetLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO ";
        case LogLevel::Warning: return "WARN ";
        case LogLevel::Error:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

std::string Logger::GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

void Logger::SetLogLevel(LogLevel level) {
    currentLevel_ = level;
}

void Logger::EnableConsoleOutput(bool enabled) {
    consoleOutputEnabled_ = enabled;
}

} // namespace Utils
} // namespace AudioVisualizer
