#ifndef AUDIOVISUALIZER_SPRITEMANAGER_H
#define AUDIOVISUALIZER_SPRITEMANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace AudioVisualizer {
namespace Graphics {

struct Sprite {
    uint32_t textureId;
    glm::vec2 position;
    glm::vec2 size;
    glm::vec2 velocity;
    float rotation;
    float rotationSpeed;
    glm::vec4 color;
    float alpha;
    bool isAnimated;
    int frameCount;
    int currentFrame;
    float frameTime;
    float frameDelay;
};

class SpriteManager {
public:
    SpriteManager();
    ~SpriteManager();

    bool Initialize();
    void Shutdown();

    void Update(float deltaTime);
    void Render();

    // Sprite management
    int LoadSprite(const std::string& imagePath);
    int LoadAnimatedSprite(const std::string& imagePath, int frameCount, float frameDelay);
    int LoadGIF(const std::string& gifPath);
    int LoadVideo(const std::string& videoPath);
    
    void AddSprite(int textureId, const glm::vec2& position, const glm::vec2& size);
    void RemoveSprite(int index);
    void ClearSprites();

    // Sprite properties
    void SetSpritePosition(int index, const glm::vec2& position);
    void SetSpriteVelocity(int index, const glm::vec2& velocity);
    void SetSpriteRotation(int index, float rotation);
    void SetSpriteRotationSpeed(int index, float speed);
    void SetSpriteAlpha(int index, float alpha);
    void SetSpriteColor(int index, const glm::vec4& color);

    // Sprite queries
    int GetSpriteCount() const { return sprites_.size(); }
    const Sprite& GetSprite(int index) const;

private:
    uint32_t LoadTextureFromFile(const std::string& path);
    void RenderSprite(const Sprite& sprite);

    std::vector<Sprite> sprites_;
    std::vector<uint32_t> textures_;

    // Shader program
    uint32_t shaderProgram_;
    uint32_t vao_;
    uint32_t vbo_;
    uint32_t ebo_;

    bool isInitialized_;
};

} // namespace Graphics
} // namespace AudioVisualizer

#endif // AUDIOVISUALIZER_SPRITEMANAGER_H
