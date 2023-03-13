#include <cinttypes>
#include <iostream>
#include <chrono>
#include <cstdlib>

#include "Vulkan/VKRenderer.hpp"
#include "OpenGL/GLRenderer.hpp"

// TODO: Consider flipping ortho height,
// make GL renderer have positive instead of negative Z, or make both from -Zfar to Zfar

int main(int argc, char **argv)
{
    GLRenderer rend("PxlRnd", 640, 480, 320, 240, true);

    SDL_Window *window = rend.GetWindowPtr();

    rend.SetBackgroundColor(0, 0, 0.2f);
    rend.SetScreenBackgroundColor(1, 1, 1);

    auto spriteBatch = rend.CreateSpriteBatch("res/tiles.png", 50000, false, false);
    auto spriteBatch2 = rend.CreateSpriteBatch("res/rgba.png", 50000, false, true);

    auto lastTime = std::chrono::high_resolution_clock::now();

    bool isRunning = true;
    int32_t frame = 0;
    while (isRunning)
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto deltaTime = static_cast<float>((currentTime - lastTime).count()) * 0.000001f;
        lastTime = currentTime;
        std::cout << deltaTime << "\n";

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
        spriteBatch.Add(0, 0, -1, 32, 32, 0, 40, 32, 32);

        // // TODO: Allow positive z values instead of negative in GL
        spriteBatch2.Clear();
        for (int32_t i = 0; i < 50'000; i++) {
            spriteBatch.Add(0, 0, -1, 64, 64, 0, 0, 64, 64);
        }
        spriteBatch2.Add(0, 32, 0, 64, 64, 0, 0, 64, 64);
        rend.DrawSpriteBatch(spriteBatch);
        rend.DrawSpriteBatch(spriteBatch2);

        rend.EndDrawing();

        frame++;
    }

    rend.DestroySpriteBatch(spriteBatch);

    return 0;
}