#pragma once

#include <unordered_map>

#ifdef EMSCRIPTEN
#include <GLES3/gl3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#else
#include <glad/glad.h>
#endif

#include "../ImageLoader.hpp"
#include "../Renderer.hpp"

struct GLModel
{
    uint32_t vao;
    uint32_t vbo;
    uint32_t ebo;
    size_t indexCount;
};

struct GLTexture
{
    uint32_t id;
};

class GLRenderer : public Renderer
{
  public:
    GLRenderer(const std::string &windowName, int32_t windowWidth, int32_t windowHeight, int32_t viewWidth,
               int32_t viewHeight, bool enableVsync = true);

    ~GLRenderer() override;
    void ResizeWindow(int32_t windowWidth, int32_t windowHeight) override;
    SDL_Window *GetWindowPtr() override;

    void SetBackgroundColor(float r, float g, float b) override;
    void SetScreenBackgroundColor(float r, float g, float b) override;
    void BeginDrawing() override;
    void EndDrawing() override;

    SpriteBatch CreateSpriteBatch(const std::string &texturePath, uint32_t maxSprites, bool smooth = false,
                                  bool enableBlending = false) override;
    void DrawSpriteBatch(SpriteBatch &spriteBatch) override;
    void DestroySpriteBatch(SpriteBatch &spriteBatch) override;

  private:
    void CheckShaderLinkError(uint32_t program);
    void CheckShaderCompileError(uint32_t shader);

    SDL_Window *window = nullptr;
    int32_t windowWidth = 0;
    int32_t windowHeight = 0;
    int32_t viewWidth = 0;
    int32_t viewHeight = 0;
    uint32_t vertexShader = 0;
    uint32_t fragmentShader = 0;
    uint32_t shaderProgram = 0;

    uint32_t screenVertexShader = 0;
    uint32_t screenFragmentShader = 0;
    uint32_t screenShaderProgram = 0;
    uint32_t screenVao = 0;
    uint32_t screenVbo = 0;
    uint32_t screenFramebuffer = 0;
    uint32_t screenTexture = 0;
    uint32_t screenDepth = 0;
    uint32_t screenTexLocation = 0;
    uint32_t screenProjLocation = 0;
    uint32_t screenViewSizeLocation = 0;
    uint32_t screenOffsetLocation = 0;

    uint32_t projLocation = 0;

    float backgroundR = 0.0f;
    float backgroundG = 0.0f;
    float backgroundB = 0.0f;
    float screenBackgroundR = 0.0f;
    float screenBackgroundG = 0.0f;
    float screenBackgroundB = 0.0f;

    GLModel spriteModel;
    std::unordered_map<uint32_t, GLTexture> spriteBatchTextures;
};
