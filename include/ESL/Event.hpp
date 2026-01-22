#pragma once
#include <iostream>
#include "Mouse.hpp"
#include "Keyboard.hpp"

namespace esl
{
	class Event
	{
		Mouse m_Mouse;
		Keyboard m_Keyboard;
		friend class Window;
	public:
		Event() = default;
		~Event();

		bool isKeyPressed(Keyboard::KeyCode key);
		bool isKeyReleased(Keyboard::KeyCode key);
		bool isMousePressed(Mouse::MouseButtonCode button);
		bool isMouseReleased(Mouse::MouseButtonCode button);
		Keyboard::KeyState getKeyState(Keyboard::KeyCode key);
		Mouse::MouseState getMouseState(Mouse::MouseButtonCode button);
	protected:
		Mouse& getMouse();
		Keyboard& getKeyboard();
		void processEvent();
	};
};
