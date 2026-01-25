#pragma once
#include "Shape.hpp"
#include "glad/glad.h"

namespace esl
{
	class Rectangle : public Shape
	{
	public:
		Rectangle(const glm::vec2& size = { 100.f, 100.f });

		void setSize(const glm::vec2& size);

		// ÖØÐ´»æÖÆÂß¼­
		virtual void draw(float right, float top) override;

	private:
		glm::vec2 m_size;

		void setupGeometry();
	};
}