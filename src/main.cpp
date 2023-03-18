#include <chrono>
#include <cinttypes>
#include <cstdlib>
#include <iostream>

#include "PxlRnd.hpp"
#include "Input.hpp"

int main(int argc, char **argv)
{
    std::unique_ptr<Renderer> rend = PxlRnd::Create("PxlRnd", 640, 480, 320, 240);

    SDL_Window *window = rend->GetWindowPtr();

    rend->SetBackgroundColor(0, 0, 0.2f);
    rend->SetScreenBackgroundColor(1, 1, 1);

    auto spriteBatch = rend->CreateSpriteBatch("res/tiles.png", 50000);

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

        rend->BeginDrawing();
        spriteBatch.Clear();

        auto sprite = Sprite{};
        sprite.width = sprite.height = sprite.texWidth = sprite.texHeight = 64;

        for (int32_t i = 0; i < 50'000; i++)
        {
            spriteBatch.Add(0, 0, 0, sprite);
        }
        rend->DrawSpriteBatch(spriteBatch);

        rend->EndDrawing();

        frame++;
    }

    rend->DestroySpriteBatch(spriteBatch);

    return 0;
}