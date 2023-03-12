#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "Error.hpp"

SDL_Surface *LoadSurface(const std::string &path);