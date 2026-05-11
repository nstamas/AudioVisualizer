#include "SpriteManager.h"
#include "utils/Logger.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// stb_image for image loading
#define STB_IMAGE_IMPLEMENTATION
#include "../../third_party/stb/stb_image.h"

namespace AudioVisualizer {
namespace Graphics {

// Simple sprite vertex shader
const char* spriteVertexShader = R"(
#version 430 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 projection;

void main() {
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

// Simple sprite fragment shader
const char* spriteFragmentShader = R"(
#version 430 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D sprite;
uniform vec4 spriteColor;
uniform float alpha;

void main() {
    vec4 texColor = texture(sprite, TexCoord);
    FragColor = texColor * spriteColor * vec4(1.0, 1.0, 1.0, alpha);
}
)";

SpriteManager::SpriteManager()
    : shaderProgram_(0)
    , vao_(0)
    , vbo_(0)
    , ebo_(0)
    , isInitialized_(false)
{
}

SpriteManager::~SpriteManager() {
    Shutdown();
}

bool SpriteManager::Initialize() {
    Utils::Logger::Info("Initializing sprite manager...");

    // Create shader program
    // TODO: Implement proper shader compilation
    // For now, this is a placeholder

    // Create sprite quad
    float vertices[] = {
        // positions   // texCoords
        0.0f, 1.0f,    0.0f, 1.0f,
        1.0f, 0.0f,    1.0f, 0.0f,
        0.0f, 0.0f,    0.0f, 0.0f,
        1.0f, 1.0f,    1.0f, 1.0f
    };

    uint32_t indices[] = {
        0, 1, 2,
        0, 3, 1
    };

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 
                         (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    isInitialized_ = true;
    Utils::Logger::Info("Sprite manager initialized");
    return true;
}

void SpriteManager::Shutdown() {
    if (!isInitialized_) {
        return;
    }

    ClearSprites();

    // Delete textures
    for (uint32_t texture : textures_) {
        glDeleteTextures(1, &texture);
    }
    textures_.clear();

    // Delete buffers
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (ebo_) glDeleteBuffers(1, &ebo_);
    if (shaderProgram_) glDeleteProgram(shaderProgram_);

    isInitialized_ = false;
}

void SpriteManager::Update(float deltaTime) {
    for (auto& sprite : sprites_) {
        // Update position based on velocity
        sprite.position += sprite.velocity * deltaTime;

        // Update rotation
        sprite.rotation += sprite.rotationSpeed * deltaTime;

        // Update animation
        if (sprite.isAnimated && sprite.frameCount > 1) {
            sprite.frameTime += deltaTime;
            if (sprite.frameTime >= sprite.frameDelay) {
                sprite.currentFrame = (sprite.currentFrame + 1) % sprite.frameCount;
                sprite.frameTime = 0.0f;
            }
        }
    }
}

void SpriteManager::Render() {
    if (!isInitialized_) {
        return;
    }

    glBindVertexArray(vao_);

    for (const auto& sprite : sprites_) {
        RenderSprite(sprite);
    }

    glBindVertexArray(0);
}

void SpriteManager::RenderSprite(const Sprite& sprite) {
    // TODO: Implement proper sprite rendering with shaders
    // This is a placeholder
    
    glBindTexture(GL_TEXTURE_2D, sprite.textureId);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

int SpriteManager::LoadSprite(const std::string& imagePath) {
    Utils::Logger::Info("Loading sprite: " + imagePath);
    
    uint32_t textureId = LoadTextureFromFile(imagePath);
    if (textureId == 0) {
        return -1;
    }

    textures_.push_back(textureId);
    return textures_.size() - 1;
}

int SpriteManager::LoadAnimatedSprite(const std::string& imagePath, 
                                     int frameCount, 
                                     float frameDelay) {
    Utils::Logger::Info("Loading animated sprite: " + imagePath);
    
    // TODO: Implement sprite sheet loading
    return LoadSprite(imagePath);
}

int SpriteManager::LoadGIF(const std::string& gifPath) {
    Utils::Logger::Info("Loading GIF: " + gifPath);
    
    // TODO: Implement GIF loading
    return -1;
}

int SpriteManager::LoadVideo(const std::string& videoPath) {
    Utils::Logger::Info("Loading video: " + videoPath);
    
    // TODO: Implement video texture loading with FFmpeg
    return -1;
}

void SpriteManager::AddSprite(int textureId, const glm::vec2& position, const glm::vec2& size) {
    if (textureId < 0 || textureId >= static_cast<int>(textures_.size())) {
        return;
    }

    Sprite sprite;
    sprite.textureId = textures_[textureId];
    sprite.position = position;
    sprite.size = size;
    sprite.velocity = glm::vec2(0.0f);
    sprite.rotation = 0.0f;
    sprite.rotationSpeed = 0.0f;
    sprite.color = glm::vec4(1.0f);
    sprite.alpha = 1.0f;
    sprite.isAnimated = false;
    sprite.frameCount = 1;
    sprite.currentFrame = 0;
    sprite.frameTime = 0.0f;
    sprite.frameDelay = 0.0f;

    sprites_.push_back(sprite);
}

void SpriteManager::RemoveSprite(int index) {
    if (index >= 0 && index < static_cast<int>(sprites_.size())) {
        sprites_.erase(sprites_.begin() + index);
    }
}

void SpriteManager::ClearSprites() {
    sprites_.clear();
}

uint32_t SpriteManager::LoadTextureFromFile(const std::string& path) {
    uint32_t textureId;
    glGenTextures(1, &textureId);

    int width, height, channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    
    if (data) {
        GLenum format = GL_RGB;
        if (channels == 1) format = GL_RED;
        else if (channels == 3) format = GL_RGB;
        else if (channels == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, 
                    format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        Utils::Logger::Info("Texture loaded: " + path);
    } else {
        Utils::Logger::Error("Failed to load texture: " + path);
        glDeleteTextures(1, &textureId);
        return 0;
    }

    return textureId;
}

void SpriteManager::SetSpritePosition(int index, const glm::vec2& position) {
    if (index >= 0 && index < static_cast<int>(sprites_.size())) {
        sprites_[index].position = position;
    }
}

void SpriteManager::SetSpriteVelocity(int index, const glm::vec2& velocity) {
    if (index >= 0 && index < static_cast<int>(sprites_.size())) {
        sprites_[index].velocity = velocity;
    }
}

void SpriteManager::SetSpriteRotation(int index, float rotation) {
    if (index >= 0 && index < static_cast<int>(sprites_.size())) {
        sprites_[index].rotation = rotation;
    }
}

void SpriteManager::SetSpriteRotationSpeed(int index, float speed) {
    if (index >= 0 && index < static_cast<int>(sprites_.size())) {
        sprites_[index].rotationSpeed = speed;
    }
}

void SpriteManager::SetSpriteAlpha(int index, float alpha) {
    if (index >= 0 && index < static_cast<int>(sprites_.size())) {
        sprites_[index].alpha = alpha;
    }
}

void SpriteManager::SetSpriteColor(int index, const glm::vec4& color) {
    if (index >= 0 && index < static_cast<int>(sprites_.size())) {
        sprites_[index].color = color;
    }
}

const Sprite& SpriteManager::GetSprite(int index) const {
    return sprites_[index];
}

} // namespace Graphics
} // namespace AudioVisualizer
