#include "Input.hpp"

void Input::UpdateStateKeyDown(KeyCode keyCode)
{
    if (heldKeys.find(keyCode) == heldKeys.end())
    {
        pressedKeys.insert(keyCode);
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