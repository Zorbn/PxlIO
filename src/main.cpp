#include <cinttypes>
#include <iostream>
#include <chrono>
#include <cstdlib>

// #include "OpenGL/GLRenderer.hpp"
#include "Vulkan/VKRenderer.hpp"

int main(int argc, char **argv)
{
    VKRenderer rend("PxlRnd", 640, 480, 320, 240, true);

    SDL_Window *window = rend.GetWindowPtr();

    rend.SetBackgroundColor(0, 0, 0.2f);
    rend.SetScreenBackgroundColor(1, 1, 1);

    auto spriteBatch = rend.CreateSpriteBatch("res/tiles.png", 50000);
    auto spriteBatch2 = rend.CreateSpriteBatch("res/test.png", 50000);

    auto lastTime = std::chrono::high_resolution_clock::now();

    bool isRunning = true;
    int32_t frame = 0;
    while (isRunning)
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto deltaTime = static_cast<float>((currentTime - lastTime).count()) * 0.000001f;
        lastTime = currentTime;
        // std::cout << deltaTime << "\n";

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_WINDOWEVENT)
            {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    rend.ResizeWindow(event.window.data1, event.window.data2);
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

        rend.BeginDrawing();
        spriteBatch.Clear();
        spriteBatch2.Clear();
        for (int32_t i = 0; i < 1; i++)
        {
            spriteBatch.Add(0, 0, 0, 32, 32, 0, 40, 32, 32, 1.0f, 0.0f, 0.0f, 0.0f);
            spriteBatch2.Add(48, 0, 0, 32, 32, 0, 40, 32, 32, 1.0f, 0.0f, 0.0f, 0.0f);
        }
        rend.DrawSpriteBatch(spriteBatch);
        rend.DrawSpriteBatch(spriteBatch2);
        rend.EndDrawing();

        frame++;
    }

    rend.DestroySpriteBatch(spriteBatch);
    rend.DestroySpriteBatch(spriteBatch2);

    return 0;
}