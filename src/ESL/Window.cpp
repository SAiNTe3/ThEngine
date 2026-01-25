#include "Window.hpp"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Sprite.hpp"
#include "Sprite3D.hpp"
#include "Cursor.hpp"
#include "Event.hpp"
#include "Text.hpp"
#include "ModernText.hpp" 
#include "Shape.hpp"
#include <chrono>
#include <thread>
#include <vector>

namespace esl
{

	Window::Window(unsigned int width, unsigned int height, const char* title, bool visiable, bool resizeable)
		: m_BackgroundColor(0.0f, 0.0f, 0.0f, 1.0f)  // 初始化背景颜色为黑色
	{
		glfwWindowHint(GLFW_VISIBLE, visiable);
		glfwWindowHint(GLFW_RESIZABLE, resizeable);
		m_Window = glfwCreateWindow(width, height, title, NULL, NULL);
		if (m_Window == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
		}
		glfwMakeContextCurrent(m_Window);
		glfwSetWindowUserPointer(m_Window, this);
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
		}
		glViewport(0, 0, width, height);
		glfwSetWindowSizeCallback(
			m_Window,
			[](GLFWwindow* window, int width, int height)
			{
				glViewport(0, 0, width, height);
			});
		glfwSetKeyCallback(m_Window, KeyEventCallback);
		glfwSetMouseButtonCallback(m_Window, MouseEventCallback);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		m_Cursor.bind(m_Window);
		m_Cursor.setStyle(Cursor::Style::Arrow);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	}
	Window::~Window()
	{
		glfwDestroyWindow(this->m_Window);
	}
	bool Window::isOpen()
	{
		return !glfwWindowShouldClose(m_Window);
	}
	void Window::display()
	{
		if (!m_Vsync)
		{
			double elapsed = glfwGetTime() - m_LastFrameTime;
			double sleepTime = m_TimePerFrame - elapsed;
			if (sleepTime > 0)
			{
				std::this_thread::sleep_for(std::chrono::duration<double>(sleepTime));
			}
		}
		glfwSwapBuffers(m_Window);
		glfwPollEvents();
		m_LastFrameTime = glfwGetTime();
	}
	void Window::clear()
	{
		glfwMakeContextCurrent(m_Window);
		glClearColor(m_BackgroundColor.r, m_BackgroundColor.g, m_BackgroundColor.b, m_BackgroundColor.a);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	void Window::pollEvents(Event& e)
	{
		m_Event.processEvent();
		e = m_Event;
	}
	glm::dvec2 Window::getCursorPosition()
	{
		glm::dvec2 pos;
		glfwGetCursorPos(m_Window, &pos.x, &pos.y);
		pos.y = this->getWindowSize().y - pos.y;
		return pos;
	}


	void Window::showWindow()
	{
		glfwShowWindow(this->m_Window);
	}
	void Window::hideWindow()
	{
		glfwHideWindow(this->m_Window);
	}
	void Window::setBackgroundColor(glm::vec4 rgba_float)
	{
		m_BackgroundColor = rgba_float;
	}
	void Window::setBackgroundColor(glm::uvec4 rgba_int)
	{
		m_BackgroundColor = {
			static_cast<float>(rgba_int.r) / 255.f,
			static_cast<float>(rgba_int.g) / 255.f,
			static_cast<float>(rgba_int.b) / 255.f,
			static_cast<float>(rgba_int.a) / 255.f
		};
	}
	void Window::setWindowSize(glm::ivec2 size)
	{
		glfwSetWindowSize(this->m_Window, size.x, size.y);
	}
	glm::ivec2 Window::getWindowSize()
	{
		glm::ivec2 size = { 0,0 };
		glfwGetWindowSize(this->m_Window, &size.x, &size.y);
		return size;
	}
	void Window::setWindowPosition(glm::ivec2 pos)
	{
		glfwSetWindowPos(this->m_Window, pos.x, pos.y);
	}
	glm::vec2 Window::getWindowPosition()
	{
		glm::ivec2 pos = { 0,0 };
		glfwGetWindowPos(this->m_Window, &pos.x, &pos.y);
		return pos;
	}
	void Window::closeWindow()
	{
		glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
	}
	void Window::setWindowTitle(const char* title)
	{
		glfwSetWindowTitle(m_Window, title);
	}

	void Window::move(glm::vec2 offset)
	{
		this->setWindowPosition(getWindowPosition() + offset);
	}
	void Window::setVSync(bool value)
	{
		m_Vsync = value;
		glfwSwapInterval(value);
		if (value)
		{
			m_Framerate = 0;
			m_TimePerFrame = 0;
		}
	}
	void Window::setFramerateLimit(double framerate)
	{
		if (framerate <= 0)
			return;
		if (m_Vsync)
		{
			return;
		}
		m_Framerate = framerate;
		m_TimePerFrame = 1.0 / framerate;
	}
	void Window::draw(Renderable& renderObject)
	{
		int width, height;
		glfwGetWindowSize(m_Window, &width, &height);
		// 检查窗口是否被最小化
		if (glfwGetWindowAttrib(m_Window, GLFW_ICONIFIED))
			return;

		// 额外检查：确保窗口尺寸有效
		if (width <= 0 || height <= 0)
			return;
		renderObject.draw(static_cast<float>(width), static_cast<float>(height));
	}

	void Window::draw(Text& text)
	{
		int width, height;
		glfwGetWindowSize(m_Window, &width, &height);
		// 检查窗口是否被最小化
		if (glfwGetWindowAttrib(m_Window, GLFW_ICONIFIED))
			return;
		// 额外检查：确保窗口尺寸有效
		if (width <= 0 || height <= 0)
			return;
		for (auto& e : text.m_SpriteVector)
		{
			this->draw(*e);
		}
	}


	void Window::KeyEventCallback(GLFWwindow* window, int key, int scancode, int action, int modes)
	{
		Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
		if (action == GLFW_PRESS)
		{
			instance->m_Event.getKeyboard().setKeyState((Keyboard::KeyCode)key, Keyboard::KeyState::PRESS);
		}
		else if (action == GLFW_RELEASE)
		{
			instance->m_Event.getKeyboard().setKeyState((Keyboard::KeyCode)key, Keyboard::KeyState::RELEASE);
		}
	}
	void Window::MouseEventCallback(GLFWwindow* window, int button, int action, int mods)
	{
		Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
		if (action == GLFW_PRESS)
		{
			instance->m_Event.getMouse().setMouseState((Mouse::MouseButtonCode)button, Mouse::MouseState::PRESS);
		}
		else if (action == GLFW_RELEASE)
		{
			instance->m_Event.getMouse().setMouseState((Mouse::MouseButtonCode)button, Mouse::MouseState::RELEASE);
		}
	}

	void Window::setCursorStyle(Cursor::Style style)
	{
		m_Cursor.setStyle(style);
	}

	void Window::setCursorIcon(const char* iconPath)
	{
		m_Cursor.reset();
		m_Cursor.loadFromFile(iconPath);
	}

	void Window::setCursorState(Cursor::State state)
	{
		m_Cursor.setState(state);
	}

	void Window::setWindowIcon(const char* iconPath)
	{
		GLFWimage icon;
		icon.pixels = stbi_load(iconPath, &icon.width, &icon.height, 0, 4);
		if (!icon.pixels)
		{
			std::cout << "Failed to load icon" << std::endl;
			return;
		}
		glfwSetWindowIcon(m_Window, 1, &icon);
	}

	GLFWwindow* Window::getWindowHandle()
	{
		return m_Window;
	}
}