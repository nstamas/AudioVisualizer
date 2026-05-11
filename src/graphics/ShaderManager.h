#ifndef AUDIOVISUALIZER_SHADERMANAGER_H
#define AUDIOVISUALIZER_SHADERMANAGER_H

#include <string>
#include <unordered_map>
#include <cstdint>

namespace AudioVisualizer {
namespace Graphics {

class ShaderManager {
public:
    ShaderManager();
    ~ShaderManager();

    bool Initialize();
    void Shutdown();

    // Shader loading
    uint32_t LoadShader(const std::string& name, 
                       const std::string& vertexPath,
                       const std::string& fragmentPath);
    uint32_t LoadShaderFromSource(const std::string& name,
                                  const std::string& vertexSource,
                                  const std::string& fragmentSource);

    // Shader management
    uint32_t GetShader(const std::string& name);
    void UseShader(const std::string& name);
    void UseShader(uint32_t shaderId);
    void DeleteShader(const std::string& name);

    // Uniform setters
    void SetBool(uint32_t shaderId, const std::string& name, bool value);
    void SetInt(uint32_t shaderId, const std::string& name, int value);
    void SetFloat(uint32_t shaderId, const std::string& name, float value);
    void SetVec2(uint32_t shaderId, const std::string& name, float x, float y);
    void SetVec3(uint32_t shaderId, const std::string& name, float x, float y, float z);
    void SetVec4(uint32_t shaderId, const std::string& name, float x, float y, float z, float w);
    void SetMat4(uint32_t shaderId, const std::string& name, const float* value);

private:
    uint32_t CompileShader(const std::string& source, uint32_t type);
    uint32_t LinkProgram(uint32_t vertexShader, uint32_t fragmentShader);
    std::string LoadShaderFile(const std::string& path);

    std::unordered_map<std::string, uint32_t> shaders_;
    bool isInitialized_;
};

} // namespace Graphics
} // namespace AudioVisualizer

#endif // AUDIOVISUALIZER_SHADERMANAGER_H
