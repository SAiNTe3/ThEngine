#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <Clock.hpp>

namespace esl
{
	Clock::Clock()
	{
		this->restart();
	}

	double Clock::getElapsedTime()
	{
		return glfwGetTime() - m_StartTime;
	}

	void Clock::restart()
	{
		m_StartTime = glfwGetTime();
		m_ElapsedTime = 0;
		m_PausedTime = 0;
		m_IsPaused = false;
	}

	void Clock::pause()
	{
		if (!m_IsPaused) {
			m_PausedTime = glfwGetTime();
			m_IsPaused = true;
		}
	}

	void Clock::resume()
	{
		if (m_IsPaused) {
			m_StartTime += (glfwGetTime() - m_PausedTime);
			m_IsPaused = false;
		}
	}

	bool Clock::isPaused()
	{
		return m_IsPaused;
	}

}
