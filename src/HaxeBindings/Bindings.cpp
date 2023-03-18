
#define HL_NAME(n) PxlRnd_##n

#include <hl.h>
#include <locale>
#include <codecvt>
#include "../PxlRnd.hpp"

static std::unique_ptr<Renderer> rend = nullptr;
static auto lastTime = std::chrono::high_resolution_clock::now();
static double deltaTime = 0.0f;
static std::unordered_map<int32_t, SpriteBatch> spriteBatches;
static int32_t lastSpriteBatchId = 0;

std::string GetHaxeString(vstring* haxeString)
{
    std::wstring wideString = haxeString->bytes;
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::string string = converter.to_bytes(wideString);

    return string;
}

HL_PRIM int HL_NAME(get_int)(int x)
{
    return x * 2;
}

HL_PRIM void HL_NAME(pxlrnd_create)(vstring* windowName, int32_t windowWidth,
    int32_t windowHeight, int32_t viewWidth, int32_t viewHeight, bool enableVsync)
{
    if (rend != nullptr)
    {
        RUNTIME_ERROR("Only one renderer can exist at a time!");
    }

    std::string name = GetHaxeString(windowName);
    rend = PxlRnd::Create(name.c_str(), windowWidth, windowHeight,
        viewWidth, viewHeight, enableVsync);
}

HL_PRIM bool HL_NAME(pxlrnd_poll_events)()
{
    auto currentTime = std::chrono::high_resolution_clock::now();
    deltaTime = static_cast<double>((currentTime - lastTime).count()) * 0.000001;
    lastTime = currentTime;

    bool isRunning = true;
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

        if (event.type == SDL_KEYDOWN)
        {
            switch (event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
                isRunning = false;
                break;
            default:
                break;
            }
        }
        else if (event.type == SDL_QUIT)
        {
            isRunning = false;
        }
    }

    return isRunning;
}

HL_PRIM double HL_NAME(pxlrnd_get_delta_time)()
{
    return deltaTime;
}

HL_PRIM void HL_NAME(pxlrnd_begin_drawing)()
{
    rend->BeginDrawing();
}

HL_PRIM void HL_NAME(pxlrnd_end_drawing)()
{
    rend->EndDrawing();
}

HL_PRIM void HL_NAME(pxlrnd_set_background_color)(double r, double g, double b)
{
    rend->SetBackgroundColor(static_cast<float>(r),
        static_cast<float>(g), static_cast<float>(b));
}

HL_PRIM void HL_NAME(pxlrnd_set_screen_background_color)(double r, double g, double b)
{
    rend->SetScreenBackgroundColor(static_cast<float>(r),
        static_cast<float>(g), static_cast<float>(b));
}

HL_PRIM int32_t HL_NAME(pxlrnd_create_sprite_batch)(vstring* texturePath,
    int32_t maxSprites, bool smooth, bool enableBlending)
{
    std::string texturePathString = GetHaxeString(texturePath);
    SpriteBatch spriteBatch = rend->CreateSpriteBatch(texturePathString,
        maxSprites, smooth, enableBlending);
    int32_t id = lastSpriteBatchId++;
    spriteBatches.insert(std::make_pair(id, spriteBatch));

    return id;
}

HL_PRIM void HL_NAME(pxlrnd_destroy_sprite_batch)(int32_t id)
{
    SpriteBatch &spriteBatch = spriteBatches.at(id);
    rend->DestroySpriteBatch(spriteBatch);
    spriteBatches.erase(id);
}

HL_PRIM void HL_NAME(pxlrnd_sprite_batch_clear)(int32_t id)
{
    SpriteBatch &spriteBatch = spriteBatches.at(id);
    spriteBatch.Clear();
}

HL_PRIM void HL_NAME(pxlrnd_sprite_batch_add)(int32_t id, float x, float y, float z,
    float width, float height, float texX, float texY, float texWidth, float texHeight,
    float originX, float originY, float rotation, float r, float g, float b, float a, float tint)
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

HL_PRIM void HL_NAME(pxlrnd_draw_sprite_batch)(int32_t id)
{
    SpriteBatch &spriteBatch = spriteBatches.at(id);
    rend->DrawSpriteBatch(spriteBatch);
}

DEFINE_PRIM(_I32, get_int, _I32);
DEFINE_PRIM(_VOID, pxlrnd_create, _STRING _I32 _I32 _I32 _I32 _BOOL);
DEFINE_PRIM(_BOOL, pxlrnd_poll_events, _NO_ARG);
DEFINE_PRIM(_F64, pxlrnd_get_delta_time, _NO_ARG);
DEFINE_PRIM(_VOID, pxlrnd_begin_drawing, _NO_ARG);
DEFINE_PRIM(_VOID, pxlrnd_end_drawing, _NO_ARG);
DEFINE_PRIM(_VOID, pxlrnd_set_background_color, _F64 _F64 _F64);
DEFINE_PRIM(_VOID, pxlrnd_set_screen_background_color, _F64 _F64 _F64);
DEFINE_PRIM(_I32, pxlrnd_create_sprite_batch, _STRING _I32 _BOOL _BOOL);
DEFINE_PRIM(_VOID, pxlrnd_destroy_sprite_batch, _I32);
DEFINE_PRIM(_VOID, pxlrnd_sprite_batch_clear, _I32);
DEFINE_PRIM(_VOID, pxlrnd_sprite_batch_add, _I32 _F32 _F32 _F32 _F32 _F32 _F32 _F32 _F32 _F32 _F32 _F32 _F32 _F32 _F32 _F32 _F32 _F32);
DEFINE_PRIM(_VOID, pxlrnd_draw_sprite_batch, _I32);