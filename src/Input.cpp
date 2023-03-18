#include "Input.hpp"

void Input::UpdateStateKeyDown(KeyCode keyCode)
{
    if (heldKeys.find(keyCode) == heldKeys.end())
    {
        pressedKeys.insert(keyCode);
        allPressedKeys.push_back(keyCode);
    }

    heldKeys.insert(keyCode);
}

void Input::UpdateStateKeyUp(KeyCode keyCode)
{
    releasedKeys.insert(keyCode);
    heldKeys.erase(keyCode);
}

void Input::Update()
{
    pressedKeys.clear();
    releasedKeys.clear();
    allPressedKeys.clear();

    pressedMouseButtons.clear();
    releasedMouseButtons.clear();
}

bool Input::IsKeyHeld(KeyCode keyCode)
{
    return heldKeys.find(keyCode) != heldKeys.end();
}

bool Input::WasKeyPressed(KeyCode keyCode)
{
    return pressedKeys.find(keyCode) != pressedKeys.end();
}

bool Input::WasKeyReleased(KeyCode keyCode)
{
    return releasedKeys.find(keyCode) != releasedKeys.end();
}

const std::vector<KeyCode> &Input::GetPressedKeys()
{
    return allPressedKeys;
}

int32_t Input::GetMouseX()
{
    int32_t x;
    SDL_GetMouseState(&x, nullptr);

    return x;
}

int32_t Input::GetMouseY()
{
    int32_t y;
    SDL_GetMouseState(nullptr, &y);

    return y;
}

void Input::UpdateStateMouseDown(MouseButton mouseButton)
{
    if (heldMouseButtons.find(mouseButton) == heldMouseButtons.end())
    {
        pressedMouseButtons.insert(mouseButton);
    }

    heldMouseButtons.insert(mouseButton);
}

void Input::UpdateStateMouseUp(MouseButton mouseButton)
{
    releasedMouseButtons.insert(mouseButton);
    heldMouseButtons.erase(mouseButton);
}

bool Input::IsMouseButtonHeld(MouseButton mouseButton)
{
    return heldMouseButtons.find(mouseButton) != heldMouseButtons.end();
}

bool Input::WasMouseButtonPressed(MouseButton mouseButton)
{
    return pressedMouseButtons.find(mouseButton) != pressedMouseButtons.end();
}

bool Input::WasMouseButtonReleased(MouseButton mouseButton)
{
    return releasedMouseButtons.find(mouseButton) != releasedMouseButtons.end();
}