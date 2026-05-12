#ifndef AUDIOVISUALIZER_FILEIO_H
#define AUDIOVISUALIZER_FILEIO_H

#include <string>
#include <vector>

namespace AudioVisualizer {
namespace Utils {

class FileIO {
public:
    // File operations
    static bool FileExists(const std::string& path);
    static bool DirectoryExists(const std::string& path);
    static bool CreateDirectory(const std::string& path);
    static bool DeleteFile(const std::string& path);
    
    // File reading
    static std::string ReadTextFile(const std::string& path);
    static std::vector<uint8_t> ReadBinaryFile(const std::string& path);
    
    // File writing
    static bool WriteTextFile(const std::string& path, const std::string& content);
    static bool WriteBinaryFile(const std::string& path, const std::vector<uint8_t>& data);
    
    // Directory operations
    static std::vector<std::string> ListDirectory(const std::string& path);
    static std::vector<std::string> ListFilesWithExtension(const std::string& path, 
                                                           const std::string& extension);
    
    // Path operations
    static std::string GetFileName(const std::string& path);
    static std::string GetFileExtension(const std::string& path);
    static std::string GetDirectoryPath(const std::string& path);
    static std::string JoinPaths(const std::string& path1, const std::string& path2);
    
    // File dialogs (platform-specific)
    static std::string OpenFileDialog(const std::string& title, 
                                      const std::string& filter = "");
    static std::string SaveFileDialog(const std::string& title, 
                                      const std::string& defaultName = "",
                                      const std::string& filter = "");
};

} // namespace Utils
} // namespace AudioVisualizer

#endif // AUDIOVISUALIZER_FILEIO_H
