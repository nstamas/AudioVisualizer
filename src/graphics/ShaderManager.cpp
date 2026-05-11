#include "ShaderManager.h"
#include "utils/Logger.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>

namespace AudioVisualizer {
namespace Graphics {

ShaderManager::ShaderManager()
    : isInitialized_(false)
{
}

ShaderManager::~ShaderManager() {
    Shutdown();
}

bool ShaderManager::Initialize() {
    Utils::Logger::Info("Initializing shader manager...");
    isInitialized_ = true;
    return true;
}

void ShaderManager::Shutdown() {
    if (!isInitialized_) {
        return;
    }

    // Delete all shaders
    for (auto& pair : shaders_) {
        glDeleteProgram(pair.second);
    }
    shaders_.clear();

    isInitialized_ = false;
}

uint32_t ShaderManager::LoadShader(const std::string& name,
                                   const std::string& vertexPath,
                                   const std::string& fragmentPath) {
    std::string vertexSource = LoadShaderFile(vertexPath);
    std::string fragmentSource = LoadShaderFile(fragmentPath);

    if (vertexSource.empty() || fragmentSource.empty()) {
        Utils::Logger::Error("Failed to load shader files");
        return 0;
    }

    return LoadShaderFromSource(name, vertexSource, fragmentSource);
}

uint32_t ShaderManager::LoadShaderFromSource(const std::string& name,
                                             const std::string& vertexSource,
                                             const std::string& fragmentSource) {
    // Compile vertex shader
    uint32_t vertexShader = CompileShader(vertexSource, GL_VERTEX_SHADER);
    if (vertexShader == 0) {
        Utils::Logger::Error("Failed to compile vertex shader: " + name);
        return 0;
    }

    // Compile fragment shader
    uint32_t fragmentShader = CompileShader(fragmentSource, GL_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        Utils::Logger::Error("Failed to compile fragment shader: " + name);
        glDeleteShader(vertexShader);
        return 0;
    }

    // Link program
    uint32_t program = LinkProgram(vertexShader, fragmentShader);
    if (program == 0) {
        Utils::Logger::Error("Failed to link shader program: " + name);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }

    // Clean up shaders (they're linked into the program now)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Store shader
    shaders_[name] = program;
    Utils::Logger::Info("Shader loaded: " + name);

    return program;
}

uint32_t ShaderManager::CompileShader(const std::string& source, uint32_t type) {
    uint32_t shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    // Check for compilation errors
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        Utils::Logger::Error("Shader compilation failed: " + std::string(infoLog));
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

uint32_t ShaderManager::LinkProgram(uint32_t vertexShader, uint32_t fragmentShader) {
    uint32_t program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // Check for linking errors
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        Utils::Logger::Error("Shader linking failed: " + std::string(infoLog));
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

std::string ShaderManager::LoadShaderFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        Utils::Logger::Error("Failed to open shader file: " + path);
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

uint32_t ShaderManager::GetShader(const std::string& name) {
    auto it = shaders_.find(name);
    if (it != shaders_.end()) {
        return it->second;
    }
    return 0;
}

void ShaderManager::UseShader(const std::string& name) {
    uint32_t shader = GetShader(name);
    if (shader != 0) {
        glUseProgram(shader);
    }
}

void ShaderManager::UseShader(uint32_t shaderId) {
    glUseProgram(shaderId);
}

void ShaderManager::DeleteShader(const std::string& name) {
    auto it = shaders_.find(name);
    if (it != shaders_.end()) {
        glDeleteProgram(it->second);
        shaders_.erase(it);
    }
}

void ShaderManager::SetBool(uint32_t shaderId, const std::string& name, bool value) {
    glUniform1i(glGetUniformLocation(shaderId, name.c_str()), static_cast<int>(value));
}

void ShaderManager::SetInt(uint32_t shaderId, const std::string& name, int value) {
    glUniform1i(glGetUniformLocation(shaderId, name.c_str()), value);
}

void ShaderManager::SetFloat(uint32_t shaderId, const std::string& name, float value) {
    glUniform1f(glGetUniformLocation(shaderId, name.c_str()), value);
}

void ShaderManager::SetVec2(uint32_t shaderId, const std::string& name, float x, float y) {
    glUniform2f(glGetUniformLocation(shaderId, name.c_str()), x, y);
}

void ShaderManager::SetVec3(uint32_t shaderId, const std::string& name, float x, float y, float z) {
    glUniform3f(glGetUniformLocation(shaderId, name.c_str()), x, y, z);
}

void ShaderManager::SetVec4(uint32_t shaderId, const std::string& name, float x, float y, float z, float w) {
    glUniform4f(glGetUniformLocation(shaderId, name.c_str()), x, y, z, w);
}

void ShaderManager::SetMat4(uint32_t shaderId, const std::string& name, const float* value) {
    glUniformMatrix4fv(glGetUniformLocation(shaderId, name.c_str()), 1, GL_FALSE, value);
}

} // namespace Graphics
} // namespace AudioVisualizer
