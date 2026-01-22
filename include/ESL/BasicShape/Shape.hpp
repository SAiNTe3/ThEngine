#pragma once
#include "Shader.hpp"
#include "Render.hpp"
namespace esl
{
	class Shape : public Renderable
	{
	protected:
		glm::vec2 m_position = { 0.f,0.f };
		float m_rotation = 0;
		glm::vec4 m_color = { 1.f,1.f,1.f,1.f };
		glm::vec2 m_scale = { 1.f,1.f };
		bool m_isShapeFilled = true;
		float m_borderWidth = 2.f;
		GLuint VAO, VBO, EBO;
		Shader* m_shapeShader;
	public:
		Shape();
		~Shape();
		void setColor(glm::vec4 color);
		void setFill(bool isFilled);
		void setPosition(glm::vec2 pos);
		void setScale(glm::vec2 scale);
		void setRotation(float angle);

		glm::vec4 getColor()const;
		bool isFilled()const;
		glm::vec2 getPosition()const;
		glm::vec2 getScale()const;
		float getRotation()const;
		void move(glm::vec2 delta);
		virtual void draw(float right, float top) = 0;
	};
}
