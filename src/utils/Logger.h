#ifndef AUDIOVISUALIZER_LOGGER_H
#define AUDIOVISUALIZER_LOGGER_H

#include <string>
#include <fstream>
#include <mutex>

namespace AudioVisualizer {
namespace Utils {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

class Logger {
public:
    static void Initialize(const std::string& logFilePath);
    static void Shutdown();

    static void Debug(const std::string& message);
    static void Info(const std::string& message);
    static void Warning(const std::string& message);
    static void Error(const std::string& message);

    static void SetLogLevel(LogLevel level);
    static void EnableConsoleOutput(bool enabled);

private:
    static void Log(LogLevel level, const std::string& message);
    static std::string GetLevelString(LogLevel level);
    static std::string GetTimestamp();

    static std::ofstream logFile_;
    static std::recursive_mutex logMutex_;
    static LogLevel currentLevel_;
    static bool consoleOutputEnabled_;
    static bool initialized_;
};

} // namespace Utils
} // namespace AudioVisualizer

#endif // AUDIOVISUALIZER_LOGGER_H
