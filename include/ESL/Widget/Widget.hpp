#pragma once
#include "BasicShape/CircleShape.hpp"

namespace esl
{

	class Widget : public CircleShape
	{
	protected:

		glm::vec2 m_size;
		std::string m_widget_name;
		
	public:
		Widget();

	};
}