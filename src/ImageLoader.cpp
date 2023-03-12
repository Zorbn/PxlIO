#include "ImageLoader.hpp"

SDL_Surface *LoadSurface(const std::string &path)
{
    SDL_Surface *loadedSurface = IMG_Load(path.c_str());

    if (!loadedSurface)
    {
        RUNTIME_ERROR(std::string("Failed to load: ") + path);
    }

    SDL_Surface *surface = SDL_ConvertSurfaceFormat(loadedSurface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(loadedSurface);

    return surface;
}