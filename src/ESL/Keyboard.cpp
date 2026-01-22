#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Keyboard.hpp"

Keyboard::KeyState Keyboard::getKeyState(KeyCode key)
{
    auto it = currentFrameState.find(key);
    return (it != currentFrameState.end()) ? it->second : RELEASE;
}
void Keyboard::setKeyState(KeyCode key, KeyState state)
{
    currentFrameState[key] = state;
}