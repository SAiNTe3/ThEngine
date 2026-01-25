#include "Button.hpp"
#include "Text.hpp"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
namespace esl
{
	Button::Button(glm::vec2 pos, glm::vec2 size):Widget()
	{
		m_position = pos;
		m_size = size;
		this->setPointCount(4);
		this->updateGeometry();
		this->setupOpenGLBuffers();
	}
	void Button::setText(const std::u32string& text)
	{
		//m_text = new Text(text);
	}
	void Button::setText(const Text& text)
	{
		//m_text = new Text(text.getText());
	}
	void Button::setPosition(glm::vec2 pos)
	{
		m_position = pos;
	}
	void Button::setSize(glm::vec2 size)
	{
		m_size = size;
		this->updateGeometry();
		this->setupOpenGLBuffers();
	}
	void Button::setColor(glm::vec4 color)
	{
		m_color = color;
	}
	void Button::onClick()
	{

	}
	void Button::onHover()
	{
	}
	void Button::updateGeometry()
	{
		m_vertices.clear();
		m_vertices.reserve(m_pointCount+1);
		m_vertices.push_back({  - m_size.x / 2, + m_size.y / 2 });
		m_vertices.push_back({  + m_size.x / 2, + m_size.y / 2 });
		m_vertices.push_back({  + m_size.x / 2, - m_size.y / 2 });
		m_vertices.push_back({  - m_size.x / 2, - m_size.y / 2 });
	}
	void Button::draw(float right, float top)
	{
		m_shapeShader->load();

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(m_position, 0.0f));
		model = glm::rotate(model, glm::radians(m_rotation), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(m_scale, 1.0f));

		glm::mat4 projection = glm::ortho(0.0f, right, 0.0f, top, -1.0f, 1.0f);

		m_shapeShader->setMat4("projection", projection);
		m_shapeShader->setMat4("model", model);
		m_shapeShader->setVec4("color", m_color);

		if (m_isShapeFilled) {
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLE_FAN, 0, static_cast<GLsizei>(m_vertices.size()));
		}
		else {
			glBindVertexArray(m_borderVAO);
			glLineWidth(m_borderWidth);
			glDrawArrays(GL_LINE_LOOP, 0, static_cast<GLsizei>(m_pointCount));
		}

		glBindVertexArray(0);
		m_shapeShader->unload();
	}
}