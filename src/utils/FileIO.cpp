#include "FileIO.h"
#include "Logger.h"
#include <fstream>
#include <sstream>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
// Undefine Windows macros that conflict with our methods
#undef CreateDirectory
#undef DeleteFile
#endif

namespace fs = std::filesystem;

namespace AudioVisualizer {
namespace Utils {

bool FileIO::FileExists(const std::string& path) {
    return fs::exists(path) && fs::is_regular_file(path);
}

bool FileIO::DirectoryExists(const std::string& path) {
    return fs::exists(path) && fs::is_directory(path);
}

bool FileIO::CreateDirectory(const std::string& path) {
    try {
        return fs::create_directories(path);
    } catch (const fs::filesystem_error& e) {
        Logger::Error("Failed to create directory: " + std::string(e.what()));
        return false;
    }
}

bool FileIO::DeleteFile(const std::string& path) {
    try {
        return fs::remove(path);
    } catch (const fs::filesystem_error& e) {
        Logger::Error("Failed to delete file: " + std::string(e.what()));
        return false;
    }
}

std::string FileIO::ReadTextFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        Logger::Error("Failed to open file for reading: " + path);
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::vector<uint8_t> FileIO::ReadBinaryFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        Logger::Error("Failed to open file for reading: " + path);
        return {};
    }

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);
    return data;
}

bool FileIO::WriteTextFile(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file.is_open()) {
        Logger::Error("Failed to open file for writing: " + path);
        return false;
    }

    file << content;
    return true;
}

bool FileIO::WriteBinaryFile(const std::string& path, const std::vector<uint8_t>& data) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        Logger::Error("Failed to open file for writing: " + path);
        return false;
    }

    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    return true;
}

std::vector<std::string> FileIO::ListDirectory(const std::string& path) {
    std::vector<std::string> files;

    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            files.push_back(entry.path().string());
        }
    } catch (const fs::filesystem_error& e) {
        Logger::Error("Failed to list directory: " + std::string(e.what()));
    }

    return files;
}

std::vector<std::string> FileIO::ListFilesWithExtension(const std::string& path, 
                                                        const std::string& extension) {
    std::vector<std::string> files;

    try {
        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (entry.is_regular_file() && entry.path().extension() == extension) {
                files.push_back(entry.path().string());
            }
        }
    } catch (const fs::filesystem_error& e) {
        Logger::Error("Failed to list files: " + std::string(e.what()));
    }

    return files;
}

std::string FileIO::GetFileName(const std::string& path) {
    fs::path p(path);
    return p.filename().string();
}

std::string FileIO::GetFileExtension(const std::string& path) {
    fs::path p(path);
    return p.extension().string();
}

std::string FileIO::GetDirectoryPath(const std::string& path) {
    fs::path p(path);
    return p.parent_path().string();
}

std::string FileIO::JoinPaths(const std::string& path1, const std::string& path2) {
    fs::path p1(path1);
    fs::path p2(path2);
    return (p1 / p2).string();
}

std::string FileIO::OpenFileDialog(const std::string& title, const std::string& filter) {
#ifdef _WIN32
    char filename[MAX_PATH] = "";
    
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = filter.empty() ? "All Files (*.*)\0*.*\0" : filter.c_str();
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = title.c_str();
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    
    if (GetOpenFileNameA(&ofn)) {
        return std::string(filename);
    }
    
    return "";
#else
    // TODO: Implement for Linux/macOS
    Logger::Warning("File dialog not yet implemented for this platform");
    return "";
#endif
}

std::string FileIO::SaveFileDialog(const std::string& title, 
                                   const std::string& defaultName,
                                   const std::string& filter) {
#ifdef _WIN32
    char filename[MAX_PATH] = "";
    if (!defaultName.empty()) {
        strncpy_s(filename, defaultName.c_str(), MAX_PATH - 1);
    }
    
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = filter.empty() ? "All Files (*.*)\0*.*\0" : filter.c_str();
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = title.c_str();
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
    
    if (GetSaveFileNameA(&ofn)) {
        return std::string(filename);
    }
    
    return "";
#else
    // TODO: Implement for Linux/macOS
    Logger::Warning("File dialog not yet implemented for this platform");
    return "";
#endif
}

} // namespace Utils
} // namespace AudioVisualizer
