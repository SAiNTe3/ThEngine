#pragma once
#include <ESL/Widget/Widget.hpp>

namespace esl
{
	class Text;

	class Button : public Widget
	{
		Text* m_text;
	public:
		Button(glm::vec2 pos, glm::vec2 size);
		void setText(const std::u32string& text);
		void setText(const Text& text);
		void setTextWidth(float width);
		void setTextColor(glm::vec3 color);
		void setPosition(glm::vec2 pos);
		void setSize(glm::vec2 size);
		void setColor(glm::vec4 color);
		virtual void onClick();
		virtual void onHover();
		virtual void updateGeometry();
		virtual void draw(float right, float top);
	};
}