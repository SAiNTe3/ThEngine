#pragma once
#include <iostream>
#include <unordered_map>

namespace esl
{
    struct Mouse
    {
        enum MouseState
        {
            PRESS,
            RELEASE
        };
        enum MouseButtonCode
        {
            MOUSE_BUTTON_1 = 0,
            MOUSE_BUTTON_2 = 1,
            MOUSE_BUTTON_3 = 2,
            MOUSE_BUTTON_4 = 3,
            MOUSE_BUTTON_5 = 4,
            MOUSE_BUTTON_6 = 5,
            MOUSE_BUTTON_7 = 6,
            MOUSE_BUTTON_8 = 7,
            MOUSE_BUTTON_LAST = MOUSE_BUTTON_8,
            MOUSE_BUTTON_LEFT = MOUSE_BUTTON_1,
            MOUSE_BUTTON_RIGHT = MOUSE_BUTTON_2,
            MOUSE_BUTTON_MIDDLE = MOUSE_BUTTON_3
        };
        std::unordered_map<int, MouseState> currentFrameState = { {MOUSE_BUTTON_1,RELEASE} };
        MouseState getMouseState(MouseButtonCode button);
        void setMouseState(MouseButtonCode button, MouseState state);
    };
}
