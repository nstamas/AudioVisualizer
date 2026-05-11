#ifndef AUDIOVISUALIZER_CONFIG_H
#define AUDIOVISUALIZER_CONFIG_H

#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace AudioVisualizer {
namespace Utils {

class Config {
public:
    Config();
    ~Config();

    bool LoadFromFile(const std::string& path);
    bool SaveToFile(const std::string& path);

    // Getters
    std::string GetString(const std::string& key, const std::string& defaultValue = "") const;
    int GetInt(const std::string& key, int defaultValue = 0) const;
    float GetFloat(const std::string& key, float defaultValue = 0.0f) const;
    bool GetBool(const std::string& key, bool defaultValue = false) const;

    // Setters
    void SetString(const std::string& key, const std::string& value);
    void SetInt(const std::string& key, int value);
    void SetFloat(const std::string& key, float value);
    void SetBool(const std::string& key, bool value);

    // Check if key exists
    bool HasKey(const std::string& key) const;

    // Clear all settings
    void Clear();

private:
    nlohmann::json data_;
    std::string filePath_;
};

} // namespace Utils
} // namespace AudioVisualizer

#endif // AUDIOVISUALIZER_CONFIG_H
