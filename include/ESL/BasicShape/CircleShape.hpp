#pragma once
#include<Shape.hpp>
#include<vector>

namespace esl
{
	class CircleShape :public Shape
	{
	protected:
		float m_radius;
		size_t m_pointCount = 30;
		std::vector<glm::vec2> m_vertices;
		GLuint m_borderVAO, m_borderVBO;
	public:
		CircleShape();
		explicit CircleShape(size_t count, float radius);
		void setRadius(float radius);
		void setPointCount(size_t count = 30);

		float getRadius()const;
		size_t getPointCount()const;
		virtual void updateGeometry();
		void setupOpenGLBuffers();
		virtual glm::vec2 getPoint(size_t index);
		virtual void draw(float right, float top)override;
		 
	};
}
