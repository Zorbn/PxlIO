#pragma once

#include <cinttypes>
#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <vector>

const uint32_t verticesPerSprite = 4;
const uint32_t valuesPerSpriteVertex = 10;
const uint32_t vertexValuesPerSprite = valuesPerSpriteVertex * verticesPerSprite;
const uint32_t indicesPerSprite = 6;

const std::vector<float> spriteVertices = {
    0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Bottom left
    1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Bottom right
    1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Top right
    0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Top left
};

const std::vector<uint32_t> spriteIndices = {0, 1, 2, 0, 2, 3};

struct Sprite
{
    float width = 0.0f;
    float height = 0.0f;
    float texX = 0.0f;
    float texY = 0.0f;
    float texWidth = 0.0f;
    float texHeight = 0.0f;
    float originX = 0.0f;
    float originY = 0.0f;
    float rotation = 0.0f;
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
    float tint = 0.0f;
};

class SpriteBatch
{
  public:
    SpriteBatch(int32_t textureWidth, int32_t textureHeight, uint32_t maxSprites, bool enableBlending = false)
        : id(nextId++), textureWidth(textureWidth), textureHeight(textureHeight), maxSprites(maxSprites),
          hasBlending(enableBlending)
    {
        inverseTextureWidth = 1.0f / textureWidth;
        inverseTextureHeight = 1.0f / textureHeight;

        vertices = std::vector<float>(maxSprites * vertexValuesPerSprite);
        indices = std::vector<uint32_t>(maxSprites * indicesPerSprite);
    }

    inline void Clear()
    {
        spriteCount = 0;
    }

    void Add(float x, float y, float depth, Sprite sprite)
    {
        if (spriteCount >= maxSprites)
        {
            return;
        }

        uint32_t vertexI = spriteCount * vertexValuesPerSprite;
        uint32_t indexI = spriteCount * indicesPerSprite;
        ++spriteCount;
        uint32_t batchVertexCount = static_cast<uint32_t>(vertexI / valuesPerSpriteVertex);

        for (size_t i = 0; i < spriteVertices.size(); i += valuesPerSpriteVertex)
        {
            float vertexX = spriteVertices[i];
            float vertexY = spriteVertices[i + 1];

            if (sprite.rotation != 0.0f)
            {
                auto rotatedVerts = glm::vec3(vertexX - sprite.originX, vertexY - sprite.originY, 0.0f) *
                                    glm::rotate(glm::mat3(1.0f), glm::radians(sprite.rotation));
                vertexX = rotatedVerts.x + sprite.originX;
                vertexY = rotatedVerts.y + sprite.originY;
            }

            vertices[vertexI + i] = x + vertexX * sprite.width;
            vertices[vertexI + i + 1] = y + vertexY * sprite.height;
            vertices[vertexI + i + 2] = depth + spriteVertices[i + 2];
            vertices[vertexI + i + 3] =
                sprite.texX * inverseTextureWidth + spriteVertices[i + 3] * sprite.texWidth * inverseTextureWidth;
            vertices[vertexI + i + 4] =
                sprite.texY * inverseTextureHeight + spriteVertices[i + 4] * sprite.texHeight * inverseTextureHeight;
            vertices[vertexI + i + 5] = sprite.r;
            vertices[vertexI + i + 6] = sprite.g;
            vertices[vertexI + i + 7] = sprite.b;
            vertices[vertexI + i + 8] = sprite.a;
            vertices[vertexI + i + 9] = sprite.tint;
        }

        for (size_t i = 0; i < spriteIndices.size(); i++)
        {
            indices[indexI + i] = (batchVertexCount + spriteIndices[i]);
        }
    }

    inline const std::vector<float> &GetVertices()
    {
        return vertices;
    }

    inline const std::vector<uint32_t> &GetIndices()
    {
        return indices;
    }

    inline uint32_t GetSpriteCount()
    {
        return spriteCount;
    }

    inline uint32_t GetId()
    {
        return id;
    }

    inline bool GetHasBlending()
    {
        return hasBlending;
    }

  private:
    inline static uint32_t nextId;
    uint32_t id = 0;
    int32_t textureWidth = 0;
    int32_t textureHeight = 0;
    float inverseTextureWidth = 0.0f;
    float inverseTextureHeight = 0.0f;
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    uint32_t maxSprites = 0;
    uint32_t spriteCount = 0;
    bool hasBlending = false;
};