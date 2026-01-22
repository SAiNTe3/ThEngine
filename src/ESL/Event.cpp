#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Event.hpp"

namespace esl
{
	Event::~Event()
	{

	}

	void Event::processEvent()
	{
		glfwPollEvents();
	}
	bool Event::isKeyPressed(Keyboard::KeyCode key)
	{
		return m_Keyboard.getKeyState(key) == Keyboard::KeyState::PRESS;
	}
	bool Event::isKeyReleased(Keyboard::KeyCode key)
	{
		return m_Keyboard.getKeyState(key) == Keyboard::KeyState::RELEASE;
	}
	bool Event::isMousePressed(Mouse::MouseButtonCode button)
	{
		return m_Mouse.getMouseState(button) == Mouse::MouseState::PRESS;
	}
	bool Event::isMouseReleased(Mouse::MouseButtonCode button)
	{
		return m_Mouse.getMouseState(button) == Mouse::MouseState::RELEASE;
	}
	Keyboard::KeyState Event::getKeyState(Keyboard::KeyCode key)
	{
		return m_Keyboard.getKeyState(key);
	}
	Mouse::MouseState Event::getMouseState(Mouse::MouseButtonCode button)
	{
		return m_Mouse.getMouseState(button);
	}

	Mouse& Event::getMouse()
	{
		return m_Mouse;
	}

	Keyboard& Event::getKeyboard()
	{
		return m_Keyboard;
	}

}
