#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <Cursor.hpp>

namespace esl
{
	Cursor::Cursor()
	{
		m_Cursor = glfwCreateStandardCursor(Style::Arrow);
	}

	Cursor::Cursor(Style style, GLFWwindow* window)
	{
		m_Cursor = glfwCreateStandardCursor(style);
		m_Window = window;
		glfwSetCursor(m_Window, m_Cursor);
	}

	Cursor::~Cursor()
	{
		this->reset();
	}
	void Cursor::loadFromFile(const char* iconPath)
	{
		GLFWimage image{};
		image.pixels = stbi_load(iconPath, &image.width, &image.height, 0, 4);
		m_Cursor = glfwCreateCursor(&image, 0, 0);
		stbi_image_free(image.pixels);
	}

	void Cursor::bind(GLFWwindow* window)
	{
		m_Window = window;
	}

	void Cursor::setState(State state)
	{
		glfwSetInputMode(m_Window, GLFW_CURSOR, state);
	}

	void Cursor::reset()
	{
		if (m_Cursor)
		{
			glfwDestroyCursor(m_Cursor);
			m_Cursor = nullptr;
		}
		glfwSetCursor(m_Window, nullptr);
	}

	void Cursor::setStyle(Style style)
	{
		this->reset();
		m_Cursor = glfwCreateStandardCursor(style);
		glfwSetCursor(m_Window, m_Cursor);
	}
}
