#include "Audio.hpp"

#ifdef EMSCRIPTEN
EM_JS(void, JsPlay, (const char *path, float volume), {
    let audio = new Audio(Module.UTF8ToString(path));
    audio.volume = volume;
    audio.play();
});
#endif

Audio::Audio(std::string path)
{
#ifdef EMSCRIPTEN
    jsAudioPath = path;
    jsAudioVolume = 1.0f;
#else
    chunk = Mix_LoadWAV(path.c_str());
    if (!chunk)
    {
        RUNTIME_ERROR(std::string("Failed to load audio: ") + path);
    }
#endif
}

void Audio::SetVolume(float volume)
{
#ifdef EMSCRIPTEN
    jsAudioVolume = volume;
#else
    Mix_VolumeChunk(chunk, static_cast<int32_t>(volume * 128.0f));
#endif
}

void Audio::Play()
{
#ifdef EMSCRIPTEN
    JsPlay(jsAudioPath.c_str(), jsAudioVolume);
#else
    if (!isMixerOpen && Mix_OpenAudio(mixerFrequency, mixerFormat,
        mixerChannelCount, mixerChunkSize) >= 0)
    {
        isMixerOpen = true;
    }

    Mix_PlayChannel(-1, chunk, 0);
#endif
}

void Audio::Destroy()
{
#ifndef EMSCRIPTEN
    Mix_FreeChunk(chunk);
#endif
}