#include "Audio.hpp"

#ifdef EMSCRIPTEN
EM_JS(void, JsOpenMixer, (), { Module.pxlrnd_mixer = {}; });

EM_JS(void, JsPlayAudio, (const char *path, float volume), {
    const pathString = Module.UTF8ToString(path);
    let audio = Module.pxlrnd_mixer[pathString];
    audio.volume = volume;
    audio.play();
});

EM_JS(void, JsLoadAudio, (const char *path), {
    const pathString = Module.UTF8ToString(path);
    Module.pxlrnd_mixer[pathString] = new Audio(pathString);
});
#endif

Audio::Audio(std::string path)
{
#ifdef EMSCRIPTEN
    jsAudioPath = path;
    jsAudioVolume = 1.0f;

    if (!isMixerOpen)
    {
        JsOpenMixer();
        isMixerOpen = true;
    }

    JsLoadAudio(path.c_str());
#else
    if (!isMixerOpen && Mix_OpenAudio(mixerFrequency, mixerFormat,
        mixerChannelCount, mixerChunkSize) >= 0)
    {
        isMixerOpen = true;
    }

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
    JsPlayAudio(jsAudioPath.c_str(), jsAudioVolume);
#else
    Mix_PlayChannel(-1, chunk, 0);
#endif
}

void Audio::Destroy()
{
#ifndef EMSCRIPTEN
    Mix_FreeChunk(chunk);
#endif
}