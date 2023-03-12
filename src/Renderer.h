#pragma once

#include <iostream>
#include <string>
#include <cinttypes>
#include <vector>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <glm/glm.hpp>

#include "SpriteBatch.h"

#define RUNTIME_ERROR(message)                                                                                                \
	std::cout << "Error in " << std::string(__FILE__).substr(SOURCE_PATH_SIZE) << "@" << __LINE__ << ": " << message << "\n"; \
	exit(1)

const float zNear = 0.0f;
const float zFar = 1000.0f;

class Renderer
{
public:
	virtual void CloseWindow() = 0;
	virtual void ResizeWindow(int32_t windowWidth, int32_t windowHeight) = 0;
	virtual SDL_Window *GetWindowPtr() = 0;

	virtual void SetBackgroundColor(float r, float g, float b) = 0;
	virtual void SetScreenBackgroundColor(float r, float g, float b) = 0;
	virtual void BeginDrawing() = 0;
	virtual void EndDrawing() = 0;
};
