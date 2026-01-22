#pragma once
#include <iostream>

#include "stbImage/stb_image.h"
struct GLFWcursor;
struct GLFWwindow;

namespace esl
{
	class Cursor
	{
		GLFWcursor* m_Cursor = nullptr;
		GLFWwindow* m_Window = nullptr;
	public:
		enum Style
		{
			Arrow = 0x00036001,
			IBeam = 0x00036002,
			Crosshair = 0x00036003,
			Hand = 0x00036004,
			HResize = 0x00036005,
			VResize = 0x00036006
		};
		enum State
		{
			Normal = 0x00034001,
			Hidden = 0x00034002,
			Disabled = 0x00034003
		};
	public:
		Cursor();
		Cursor(Style style, GLFWwindow* window);
		~Cursor();
		void loadFromFile(const char* iconPath);
		void bind(GLFWwindow* window);
		void setStyle(Style style);
		void setState(State state);
		void reset();
	};
}
