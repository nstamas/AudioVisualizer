#include "Config.h"
#include "Logger.h"
#include <fstream>

namespace AudioVisualizer {
namespace Utils {

Config::Config()
    : data_(nlohmann::json::object())
{
}

Config::~Config() {
}

bool Config::LoadFromFile(const std::string& path) {
    filePath_ = path;

    std::ifstream file(path);
    if (!file.is_open()) {
        Logger::Warning("Config file not found, using defaults: " + path);
        return false;
    }

    try {
        file >> data_;
        Logger::Info("Configuration loaded from: " + path);
        return true;
    } catch (const nlohmann::json::exception& e) {
        Logger::Error("Failed to parse config file: " + std::string(e.what()));
        return false;
    }
}

bool Config::SaveToFile(const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) {
        Logger::Error("Failed to open config file for writing: " + path);
        return false;
    }

    try {
        file << data_.dump(4); // Pretty print with 4 spaces
        Logger::Info("Configuration saved to: " + path);
        return true;
    } catch (const nlohmann::json::exception& e) {
        Logger::Error("Failed to save config file: " + std::string(e.what()));
        return false;
    }
}

std::string Config::GetString(const std::string& key, const std::string& defaultValue) const {
    try {
        // Support nested keys with dot notation (e.g., "audio.sample_rate")
        auto keys = nlohmann::json::json_pointer("/" + key);
        keys = nlohmann::json::json_pointer(key);
        
        // Split key by dots
        size_t pos = 0;
        std::string currentKey = key;
        nlohmann::json current = data_;
        
        while ((pos = currentKey.find('.')) != std::string::npos) {
            std::string part = currentKey.substr(0, pos);
            if (current.contains(part)) {
                current = current[part];
            } else {
                return defaultValue;
            }
            currentKey = currentKey.substr(pos + 1);
        }
        
        if (current.contains(currentKey)) {
            return current[currentKey].get<std::string>();
        }
    } catch (const nlohmann::json::exception&) {
        // Fall through to return default
    }
    
    return defaultValue;
}

int Config::GetInt(const std::string& key, int defaultValue) const {
    try {
        size_t pos = 0;
        std::string currentKey = key;
        nlohmann::json current = data_;
        
        while ((pos = currentKey.find('.')) != std::string::npos) {
            std::string part = currentKey.substr(0, pos);
            if (current.contains(part)) {
                current = current[part];
            } else {
                return defaultValue;
            }
            currentKey = currentKey.substr(pos + 1);
        }
        
        if (current.contains(currentKey)) {
            return current[currentKey].get<int>();
        }
    } catch (const nlohmann::json::exception&) {
        // Fall through to return default
    }
    
    return defaultValue;
}

float Config::GetFloat(const std::string& key, float defaultValue) const {
    try {
        size_t pos = 0;
        std::string currentKey = key;
        nlohmann::json current = data_;
        
        while ((pos = currentKey.find('.')) != std::string::npos) {
            std::string part = currentKey.substr(0, pos);
            if (current.contains(part)) {
                current = current[part];
            } else {
                return defaultValue;
            }
            currentKey = currentKey.substr(pos + 1);
        }
        
        if (current.contains(currentKey)) {
            return current[currentKey].get<float>();
        }
    } catch (const nlohmann::json::exception&) {
        // Fall through to return default
    }
    
    return defaultValue;
}

bool Config::GetBool(const std::string& key, bool defaultValue) const {
    try {
        size_t pos = 0;
        std::string currentKey = key;
        nlohmann::json current = data_;
        
        while ((pos = currentKey.find('.')) != std::string::npos) {
            std::string part = currentKey.substr(0, pos);
            if (current.contains(part)) {
                current = current[part];
            } else {
                return defaultValue;
            }
            currentKey = currentKey.substr(pos + 1);
        }
        
        if (current.contains(currentKey)) {
            return current[currentKey].get<bool>();
        }
    } catch (const nlohmann::json::exception&) {
        // Fall through to return default
    }
    
    return defaultValue;
}

void Config::SetString(const std::string& key, const std::string& value) {
    // Handle nested keys
    size_t pos = 0;
    std::string currentKey = key;
    nlohmann::json* current = &data_;
    
    while ((pos = currentKey.find('.')) != std::string::npos) {
        std::string part = currentKey.substr(0, pos);
        if (!current->contains(part)) {
            (*current)[part] = nlohmann::json::object();
        }
        current = &(*current)[part];
        currentKey = currentKey.substr(pos + 1);
    }
    
    (*current)[currentKey] = value;
}

void Config::SetInt(const std::string& key, int value) {
    size_t pos = 0;
    std::string currentKey = key;
    nlohmann::json* current = &data_;
    
    while ((pos = currentKey.find('.')) != std::string::npos) {
        std::string part = currentKey.substr(0, pos);
        if (!current->contains(part)) {
            (*current)[part] = nlohmann::json::object();
        }
        current = &(*current)[part];
        currentKey = currentKey.substr(pos + 1);
    }
    
    (*current)[currentKey] = value;
}

void Config::SetFloat(const std::string& key, float value) {
    size_t pos = 0;
    std::string currentKey = key;
    nlohmann::json* current = &data_;
    
    while ((pos = currentKey.find('.')) != std::string::npos) {
        std::string part = currentKey.substr(0, pos);
        if (!current->contains(part)) {
            (*current)[part] = nlohmann::json::object();
        }
        current = &(*current)[part];
        currentKey = currentKey.substr(pos + 1);
    }
    
    (*current)[currentKey] = value;
}

void Config::SetBool(const std::string& key, bool value) {
    size_t pos = 0;
    std::string currentKey = key;
    nlohmann::json* current = &data_;
    
    while ((pos = currentKey.find('.')) != std::string::npos) {
        std::string part = currentKey.substr(0, pos);
        if (!current->contains(part)) {
            (*current)[part] = nlohmann::json::object();
        }
        current = &(*current)[part];
        currentKey = currentKey.substr(pos + 1);
    }
    
    (*current)[currentKey] = value;
}

bool Config::HasKey(const std::string& key) const {
    try {
        size_t pos = 0;
        std::string currentKey = key;
        nlohmann::json current = data_;
        
        while ((pos = currentKey.find('.')) != std::string::npos) {
            std::string part = currentKey.substr(0, pos);
            if (current.contains(part)) {
                current = current[part];
            } else {
                return false;
            }
            currentKey = currentKey.substr(pos + 1);
        }
        
        return current.contains(currentKey);
    } catch (const nlohmann::json::exception&) {
        return false;
    }
}

void Config::Clear() {
    data_ = nlohmann::json::object();
}

} // namespace Utils
} // namespace AudioVisualizer
