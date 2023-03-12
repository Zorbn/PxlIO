#pragma once

#include <cinttypes>
#include <vector>

const uint32_t verticesPerSprite = 4;
const uint32_t valuesPerSpriteVertex = 9;
const uint32_t vertexValuesPerSprite = valuesPerSpriteVertex * verticesPerSprite;
const uint32_t indicesPerSprite = 6;

const std::vector<float> spriteVertices = {
    0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Bottom left
    1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Bottom right
    1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Top right
    0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Top left
};

const std::vector<uint32_t> spriteIndices = {0, 1, 2, 0, 2, 3};

class SpriteBatch
{
public:
    SpriteBatch(int32_t textureWidth, int32_t textureHeight, uint32_t maxSprites)
        : id(nextId++), textureWidth(textureWidth), textureHeight(textureHeight), maxSprites(maxSprites)
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

    void Add(float x, float y, float depth, float width,
             float height, float texX, float texY,
             float texWidth, float texHeight,
             float r, float g, float b, float a)
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
            vertices[vertexI + i] = x + spriteVertices[i] * width;
            vertices[vertexI + i + 1] = y + spriteVertices[i + 1] * height;
            vertices[vertexI + i + 2] = depth + spriteVertices[i + 2];
            vertices[vertexI + i + 3] = texX * inverseTextureWidth +
                                        spriteVertices[i + 3] * texWidth * inverseTextureWidth;
            vertices[vertexI + i + 4] = texY * inverseTextureHeight +
                                        spriteVertices[i + 4] * texHeight * inverseTextureHeight;
            vertices[vertexI + i + 5] = r;
            vertices[vertexI + i + 6] = g;
            vertices[vertexI + i + 7] = b;
            vertices[vertexI + i + 8] = a;
        }

        for (size_t i = 0; i < spriteIndices.size(); i++)
        {
            indices[indexI + i] = (batchVertexCount + spriteIndices[i]);
        }
    }

    inline const std::vector<float> &Vertices()
    {
        return vertices;
    }

    inline const std::vector<uint32_t> &Indices()
    {
        return indices;
    }

    inline uint32_t SpriteCount()
    {
        return spriteCount;
    }

    inline uint32_t Id()
    {
        return id;
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
};