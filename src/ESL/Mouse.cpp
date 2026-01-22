#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Mouse.hpp"

namespace esl
{
    Mouse::MouseState Mouse::getMouseState(MouseButtonCode button)
    {
        auto it = currentFrameState.find(button);
        return (it != currentFrameState.end()) ? it->second : RELEASE;
    }
    void Mouse::setMouseState(MouseButtonCode key, MouseState state)
    {
        currentFrameState[key] = state;
    }
}
