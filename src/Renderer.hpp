#pragma once

#include <cinttypes>
#include <iostream>
#include <string>
#include <vector>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <glm/glm.hpp>

#include "Error.hpp"
#include "SpriteBatch.hpp"

const float zMax = 1000.0f;

struct ViewTransform
{
    float scaledViewWidth;
    float scaledViewHeight;
    float offsetX;
    float offsetY;
};

class Renderer
{
  public:
    virtual ~Renderer() {}

    virtual void ResizeWindow(int32_t windowWidth, int32_t windowHeight) = 0;
    virtual SDL_Window *GetWindowPtr() = 0;

    virtual void SetBackgroundColor(float r, float g, float b) = 0;
    virtual void SetScreenBackgroundColor(float r, float g, float b) = 0;
    virtual void BeginDrawing() = 0;
    virtual void EndDrawing() = 0;

    virtual SpriteBatch CreateSpriteBatch(const std::string &texturePath, uint32_t maxSprites, bool smooth = false,
                                          bool enableBlending = false) = 0;
    virtual void DrawSpriteBatch(SpriteBatch &spriteBatch) = 0;
    virtual void DestroySpriteBatch(SpriteBatch &spriteBatch) = 0;

    static ViewTransform CalcViewTransform(int32_t windowWidth, int32_t windowHeight, int32_t viewWidth,
                                           int32_t viewHeight)
    {
        float widthRatio = windowWidth / static_cast<float>(viewWidth);
        float heightRatio = windowHeight / static_cast<float>(viewHeight);
        float scale = heightRatio;

        if (widthRatio < heightRatio)
        {
            scale = widthRatio;
        }

        if (scale > 1.0f)
        {
            scale = glm::floor(scale);
        }

        float scaledViewWidth = viewWidth * scale;
        float scaledViewHeight = viewHeight * scale;
        float offsetX = (windowWidth - scaledViewWidth) * 0.5f;
        float offsetY = (windowHeight - scaledViewHeight) * 0.5f;

        return ViewTransform{
            scaledViewWidth,
            scaledViewHeight,
            offsetX,
            offsetY,
        };
    }
};
