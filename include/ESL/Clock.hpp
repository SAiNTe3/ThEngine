#pragma once
#include <iostream>

namespace esl
{
	class Clock {
		double m_StartTime;
		double m_ElapsedTime = 0;
		double m_PausedTime = 0;
		bool m_IsPaused = false;
	public:
		Clock();
		~Clock() = default;
		double getElapsedTime();
		void restart();
		void pause();
		void resume();
		bool isPaused();
	};
};
