#pragma once

#include "Error.hpp"

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <iostream>
#include <cinttypes>

const int32_t mixerFrequency = 44100;
const uint16_t mixerFormat = MIX_DEFAULT_FORMAT;
const int32_t mixerChannelCount = 2;
const int32_t mixerChunkSize = 2048;

class Audio
{
public:
    Audio(std::string path);
    void SetVolume(float volume);
    void Play();
    void Destroy();

  private:
    static inline bool isMixerOpen = false;

#ifdef EMSCRIPTEN
    std::string jsAudioPath;
    float jsAudioVolume;
#else
    Mix_Chunk *chunk;
#endif
};