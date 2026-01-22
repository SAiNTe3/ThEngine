#include<Shape.hpp>

namespace esl
{

	Shape::Shape()
	{
		const std::string vertexShader = R"(
		#version 460 core
		layout(location = 0) in vec2 aPos;
		uniform mat4 projection;
		uniform mat4 model;
		void main() {
			gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
		}
	)";

		const std::string fragmentShader = R"(
		#version 460 core
		out vec4 fragColor;
		uniform vec4 color;
		void main() {
			fragColor = color;
		}
	)";
		m_shapeShader = new Shader(vertexShader, fragmentShader);
	}

	Shape::~Shape()
	{
		if (this->m_shapeShader)
		{
			delete m_shapeShader;
		}
	}

	void Shape::setColor(glm::vec4 color)
	{
		this->m_color = color;
	}

	void Shape::setFill(bool isFilled)
	{
		this->m_isShapeFilled = isFilled;
	}

	void Shape::setPosition(glm::vec2 pos)
	{
		this->m_position = pos;
	}

	void Shape::setScale(glm::vec2 scale)
	{
		this->m_scale = scale;
	}

	void Shape::setRotation(float angle)
	{
		this->m_rotation = angle;
	}

	glm::vec4 Shape::getColor() const
	{
		return this->m_color;
	}

	bool Shape::isFilled() const
	{
		return this->m_isShapeFilled;
	}

	glm::vec2 Shape::getPosition() const
	{
		return this->m_position;
	}

	glm::vec2 Shape::getScale() const
	{
		return this->m_scale;
	}

	float Shape::getRotation() const
	{
		return this->m_rotation;
	}

	void Shape::move(glm::vec2 delta)
	{
		this->m_position += delta;
	}
}