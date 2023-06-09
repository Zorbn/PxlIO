
#define HL_NAME(n) PxlIO_##n

#include <codecvt>
#include <hl.h>
#include <locale>
#include "../PxlIO.hpp"
#include "../Input.hpp"
#include "../Audio.hpp"

static std::unique_ptr<Renderer> rend = nullptr;
static bool isRunning = false;

static Input input;

static auto lastTime = std::chrono::high_resolution_clock::now();
static float deltaTime = 0.0f;

static std::unordered_map<int32_t, SpriteBatch> spriteBatches;
static int32_t lastSpriteBatchId = 0;

static std::unordered_map<int32_t, Audio> audios;
static int32_t lastAudioId = 0;

std::string GetHaxeString(vstring *haxeString)
{
    std::wstring wideString = haxeString->bytes;
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::string string = converter.to_bytes(wideString);

    return string;
}

HL_PRIM void HL_NAME(pxlio_create)(vstring *windowName, int32_t windowWidth, int32_t windowHeight, int32_t viewWidth,
                                    int32_t viewHeight, bool enableVsync)
{
    if (rend)
    {
        hl_error("Only one renderer can exist at a time!");
        return;
    }

    std::string name = GetHaxeString(windowName);
    rend = PxlIO::Create(name.c_str(), windowWidth, windowHeight, viewWidth, viewHeight, enableVsync);
    isRunning = true;
}

HL_PRIM bool HL_NAME(pxlio_poll_events)()
{
    if (!rend)
    {
        hl_error("The renderer isn't active!");
        return false;
    }

    auto currentTime = std::chrono::high_resolution_clock::now();
    deltaTime = static_cast<float>((currentTime - lastTime).count()) * 0.000000001f;
    lastTime = currentTime;

    input.Update();

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_WINDOWEVENT)
        {
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                rend->ResizeWindow(event.window.data1, event.window.data2);
            }
        }

        switch (event.type)
        {
        case SDL_KEYDOWN:
            input.UpdateStateKeyDown((KeyCode)event.key.keysym.sym);
            break;
        case SDL_KEYUP:
            input.UpdateStateKeyUp((KeyCode)event.key.keysym.sym);
            break;
        case SDL_MOUSEBUTTONDOWN:
            input.UpdateStateMouseDown((MouseButton)event.button.button);
            break;
        case SDL_MOUSEBUTTONUP:
            input.UpdateStateMouseUp((MouseButton)event.button.button);
            break;
        case SDL_QUIT:
            isRunning = false;
            break;
        }
    }

    if (!isRunning)
    {
        rend.reset();
    }

    return isRunning;
}

HL_PRIM float HL_NAME(pxlio_get_delta_time)()
{
    if (!rend)
    {
        hl_error("The renderer isn't active!");
        return 0.0f;
    }

    return deltaTime;
}

HL_PRIM void HL_NAME(pxlio_begin_drawing)()
{
    if (!rend)
    {
        hl_error("The renderer isn't active!");
        return;
    }

    rend->BeginDrawing();
}

HL_PRIM void HL_NAME(pxlio_end_drawing)()
{
    if (!rend)
    {
        hl_error("The renderer isn't active!");
        return;
    }

    rend->EndDrawing();
}

HL_PRIM void HL_NAME(pxlio_set_background_color)(float r, float g, float b)
{
    if (!rend)
    {
        hl_error("The renderer isn't active!");
        return;
    }

    rend->SetBackgroundColor(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b));
}

HL_PRIM void HL_NAME(pxlio_set_screen_background_color)(float r, float g, float b)
{
    if (!rend)
    {
        hl_error("The renderer isn't active!");
        return;
    }

    rend->SetScreenBackgroundColor(static_cast<float>(r), static_cast<float>(g), static_cast<float>(b));
}

HL_PRIM int32_t HL_NAME(pxlio_create_sprite_batch)(vstring *texturePath, int32_t maxSprites, bool smooth,
                                                    bool enableBlending)
{
    if (!rend)
    {
        hl_error("The renderer isn't active!");
        return -1;
    }

    std::string texturePathString = GetHaxeString(texturePath);
    SpriteBatch spriteBatch = rend->CreateSpriteBatch(texturePathString, maxSprites, smooth, enableBlending);
    int32_t id = lastSpriteBatchId++;
    spriteBatches.insert(std::make_pair(id, spriteBatch));

    return id;
}

HL_PRIM void HL_NAME(pxlio_destroy_sprite_batch)(int32_t id)
{
    if (!rend)
    {
        hl_error("The renderer isn't active!");
        return;
    }

    SpriteBatch &spriteBatch = spriteBatches.at(id);
    rend->DestroySpriteBatch(spriteBatch);
    spriteBatches.erase(id);
}

HL_PRIM void HL_NAME(pxlio_sprite_batch_clear)(int32_t id)
{
    SpriteBatch &spriteBatch = spriteBatches.at(id);
    spriteBatch.Clear();
}

HL_PRIM void HL_NAME(pxlio_sprite_batch_add)(int32_t id, float x, float y, float z, float width, float height,
                                              float texX, float texY, float texWidth, float texHeight, float originX,
                                              float originY, float rotation, float r, float g, float b, float a,
                                              float tint)
{
    SpriteBatch &spriteBatch = spriteBatches.at(id);

    auto sprite = Sprite{};
    sprite.width = width;
    sprite.height = height;
    sprite.texX = texX;
    sprite.texY = texY;
    sprite.texWidth = texWidth;
    sprite.texHeight = texHeight;
    sprite.originX = originX;
    sprite.originY = originY;
    sprite.rotation = rotation;
    sprite.r = r;
    sprite.g = g;
    sprite.b = b;
    sprite.a = a;
    sprite.tint = tint;

    spriteBatch.Add(x, y, z, sprite);
}

HL_PRIM void HL_NAME(pxlio_draw_sprite_batch)(int32_t id)
{
    if (!rend)
    {
        hl_error("The renderer isn't active!");
        return;
    }

    SpriteBatch &spriteBatch = spriteBatches.at(id);
    rend->DrawSpriteBatch(spriteBatch);
}

HL_PRIM bool HL_NAME(pxlio_is_key_held)(int32_t keyNumber)
{
    return input.IsKeyHeld((KeyCode)keyNumber);
}

HL_PRIM bool HL_NAME(pxlio_was_key_pressed)(int32_t keyNumber)
{
    return input.WasKeyPressed((KeyCode)keyNumber);
}

HL_PRIM bool HL_NAME(pxlio_was_key_released)(int32_t keyNumber)
{
    return input.WasKeyReleased((KeyCode)keyNumber);
}

HL_PRIM vbyte *HL_NAME(pxlio_get_pressed_keys)()
{
    auto &pressedKeys = input.GetPressedKeys();

    vbyte *buffer = hl_alloc_bytes(sizeof(int32_t) * static_cast<int32_t>(pressedKeys.size() + 1));
    int32_t *intBuffer = reinterpret_cast<int32_t *>(buffer);
    intBuffer[0] = static_cast<int32_t>(pressedKeys.size());
    for (int32_t i = 0; i < pressedKeys.size(); i++)
    {
        intBuffer[i + 1] = pressedKeys[i];
    }

    return buffer;
}

HL_PRIM int32_t HL_NAME(pxlio_get_mouse_x)()
{
    return input.GetMouseX();
}

HL_PRIM int32_t HL_NAME(pxlio_get_mouse_y)()
{
    return input.GetMouseY();
}

HL_PRIM bool HL_NAME(pxlio_is_mouse_button_held)(int32_t mouseButtonNumber)
{
    return input.IsMouseButtonHeld((MouseButton)mouseButtonNumber);
}

HL_PRIM bool HL_NAME(pxlio_was_mouse_button_pressed)(int32_t mouseButtonNumber)
{
    return input.WasMouseButtonPressed((MouseButton)mouseButtonNumber);
}

HL_PRIM bool HL_NAME(pxlio_was_mouse_button_released)(int32_t mouseButtonNumber)
{
    return input.WasMouseButtonReleased((MouseButton)mouseButtonNumber);
}

HL_PRIM void HL_NAME(pxlio_close)()
{
    isRunning = false;
}

HL_PRIM int32_t HL_NAME(pxlio_audio_constructor)(vstring *path)
{
    std::string pathString = GetHaxeString(path);
    Audio audio = Audio(pathString);
    int32_t id = lastAudioId++;
    audios.insert(std::make_pair(id, audio));

    return id;
}

HL_PRIM void HL_NAME(pxlio_audio_set_volume)(int32_t id, float volume)
{
    Audio &audio = audios.at(id);
    audio.SetVolume(volume);
}

HL_PRIM void HL_NAME(pxlio_audio_play)(int32_t id)
{
    Audio &audio = audios.at(id);
    audio.Play();
}

HL_PRIM void HL_NAME(pxlio_audio_destroy)(int32_t id)
{
    Audio &audio = audios.at(id);
    audio.Destroy();
    audios.erase(id);
}

DEFINE_PRIM(_VOID, pxlio_create, _STRING _I32 _I32 _I32 _I32 _BOOL);
DEFINE_PRIM(_BOOL, pxlio_poll_events, _NO_ARG);
DEFINE_PRIM(_F32, pxlio_get_delta_time, _NO_ARG);
DEFINE_PRIM(_VOID, pxlio_begin_drawing, _NO_ARG);
DEFINE_PRIM(_VOID, pxlio_end_drawing, _NO_ARG);
DEFINE_PRIM(_VOID, pxlio_set_background_color, _F32 _F32 _F32);
DEFINE_PRIM(_VOID, pxlio_set_screen_background_color, _F32 _F32 _F32);
DEFINE_PRIM(_I32, pxlio_create_sprite_batch, _STRING _I32 _BOOL _BOOL);
DEFINE_PRIM(_VOID, pxlio_destroy_sprite_batch, _I32);
DEFINE_PRIM(_VOID, pxlio_sprite_batch_clear, _I32);
DEFINE_PRIM(_VOID, pxlio_sprite_batch_add,
            _I32 _F32 _F32 _F32 _F32 _F32 _F32 _F32 _F32 _F32 _F32 _F32 _F32 _F32 _F32 _F32 _F32 _F32);
DEFINE_PRIM(_VOID, pxlio_draw_sprite_batch, _I32);
DEFINE_PRIM(_BOOL, pxlio_is_key_held, _I32);
DEFINE_PRIM(_BOOL, pxlio_was_key_pressed, _I32);
DEFINE_PRIM(_BOOL, pxlio_was_key_released, _I32);
DEFINE_PRIM(_BYTES, pxlio_get_pressed_keys, _NO_ARG);
DEFINE_PRIM(_I32, pxlio_get_mouse_x, _NO_ARG);
DEFINE_PRIM(_I32, pxlio_get_mouse_y, _NO_ARG);
DEFINE_PRIM(_BOOL, pxlio_is_mouse_button_held, _I32);
DEFINE_PRIM(_BOOL, pxlio_was_mouse_button_pressed, _I32);
DEFINE_PRIM(_BOOL, pxlio_was_mouse_button_released, _I32);
DEFINE_PRIM(_VOID, pxlio_close, _NO_ARG);
DEFINE_PRIM(_I32, pxlio_audio_constructor, _STRING);
DEFINE_PRIM(_VOID, pxlio_audio_set_volume, _I32 _F32);
DEFINE_PRIM(_VOID, pxlio_audio_play, _I32);
DEFINE_PRIM(_VOID, pxlio_audio_destroy, _I32);