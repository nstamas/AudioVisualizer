#ifndef AUDIOVISUALIZER_LOGGER_H
#define AUDIOVISUALIZER_LOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <vector>
#include <deque>

namespace AudioVisualizer {
namespace Utils {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

struct LogEntry {
    LogLevel level;
    std::string timestamp;
    std::string message;
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
    
    // Get recent log entries for UI display
    static std::vector<LogEntry> GetRecentEntries(size_t maxCount = 1000);

private:
    static void Log(LogLevel level, const std::string& message);
    static std::string GetLevelString(LogLevel level);
    static std::string GetTimestamp();

    static std::ofstream logFile_;
    static std::recursive_mutex logMutex_;
    static LogLevel currentLevel_;
    static bool consoleOutputEnabled_;
    static bool initialized_;
    static std::deque<LogEntry> recentEntries_;
    static const size_t maxRecentEntries_ = 1000;
};

} // namespace Utils
} // namespace AudioVisualizer

#endif // AUDIOVISUALIZER_LOGGER_H
