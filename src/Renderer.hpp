#pragma once

#include <iostream>
#include <string>
#include <cinttypes>
#include <vector>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <glm/glm.hpp>

#include "Error.hpp"
#include "SpriteBatch.hpp"

const float zNear = 0.0f;
const float zFar = 1000.0f;

class Renderer
{
public:
	virtual void ResizeWindow(int32_t windowWidth, int32_t windowHeight) = 0;
	virtual SDL_Window *GetWindowPtr() = 0;

	virtual void SetBackgroundColor(float r, float g, float b) = 0;
	virtual void SetScreenBackgroundColor(float r, float g, float b) = 0;
	virtual void BeginDrawing() = 0;
	virtual void EndDrawing() = 0;

	virtual SpriteBatch CreateSpriteBatch(const std::string &texturePath, uint32_t maxSprites) = 0;
	virtual void DrawSpriteBatch(SpriteBatch &spriteBatch) = 0;
	virtual void DestroySpriteBatch(SpriteBatch &spriteBatch) = 0;
};
