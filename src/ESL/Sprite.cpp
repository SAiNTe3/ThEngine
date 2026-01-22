#include"glad/glad.h"
#include"GLFW/glfw3.h"
#include "Sprite.hpp"

namespace esl
{
	Sprite::Sprite()
	{
		setup();
	}
	Sprite::Sprite(Texture* texture)
	{
		setup();
		m_Texture = texture;
		m_Size = { texture->getWidth(), texture->getHeight() };
	}
	void Sprite::setup()
	{
		glGenVertexArrays(1, &m_VAO);
		glBindVertexArray(m_VAO);
		unsigned int indices[] = { 0, 1, 2, 1, 2, 3 };
		float vertices[] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
			-0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
			0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f };
		glGenBuffers(1, &m_VBO);
		glGenBuffers(1, &m_EBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		const std::string vstring = {
			"#version 460 core\n"
			"layout(location = 0) in vec3 aPos;\n"
			"layout(location = 2) in vec2 aUV;\n"
			"out vec2 uv;\n"
			"uniform mat4 transform;\n"
			"uniform mat4 projection;\n"
			"uniform mat4 view;\n"
			"uniform vec2 uvScale;\n"
			"void main() {\n"
			"gl_Position = projection * view * transform * vec4(aPos, 1.0);\n"
			"uv = aUV * uvScale;\n"
			"}\0" };
		const std::string fstring = {
			"#version 460 core\n"
			"in vec2 uv;\n"
			"out vec4 fragColor;\n"
			"uniform sampler2D sampler;\n"
			"uniform vec4 spriteColor;\n"
			"void main() {\n"
			"fragColor = texture(sampler,uv)*spriteColor;\n"
			"}\0" };
		m_Shader = new Shader(vstring, fstring);
		m_Shader->setInt("sampler", 0);
	}

	void Sprite::setPosition(glm::vec2 pos)
	{
		m_Position = { pos.x, pos.y, m_Position.z };
	}
	void Sprite::setTexture(Texture* texture)
	{
		m_Texture = texture;
		m_Size = { texture->getWidth(), texture->getHeight() };
	}
	Texture* Sprite::getTexture() const
	{
		return m_Texture;
	}
	glm::vec2 Sprite::getPosition() const
	{
		return { m_Position.x, m_Position.y };
	}
	void Sprite::setRotation(float angle)
	{
		m_Rotation = { angle };
	}
	float Sprite::getRotation() const
	{
		return m_Rotation;
	}
	void Sprite::setColor(glm::vec4 rgba_float)
	{
		m_Color = rgba_float;
	}
	void Sprite::setColor(glm::uvec4 rgba_int)
	{
		m_Color = {
			static_cast<float>(rgba_int.r) / 255.f,
			static_cast<float>(rgba_int.g) / 255.f,
			static_cast<float>(rgba_int.b) / 255.f,
			static_cast<float>(rgba_int.a) / 255.f,
		};
	}
	glm::vec4 Sprite::getColor() const
	{
		return m_Color;
	}
	void Sprite::setScale(glm::vec2 scale)
	{
		m_Scale = scale;
	}
	glm::vec2 Sprite::getScale() const
	{
		return m_Scale;
	}

	void Sprite::setOrigin(glm::vec2 pos)
	{
		m_Origin = pos;
	}
	glm::vec2 Sprite::getOrigin() const
	{
		return m_Origin;
	}
	glm::vec2 Sprite::getGlobalSize()
	{
		return m_Size * m_Scale * m_RectScale;
	}
	glm::vec2 Sprite::getLocalSize()
	{
		return m_Size * m_RectScale;
	}

	bool Sprite::getAvailable() const
	{
		return m_available;
	}

	void Sprite::move(glm::vec2 distance)
	{
		m_Position += glm::vec3{ distance.x, distance.y, 0 };
	}
	void Sprite::setTextureRect(glm::vec2 pos, glm::vec2 size)
	{
		float textureWidth = static_cast<float>(m_Texture->getWidth());
		float textureHeight = static_cast<float>(m_Texture->getHeight());

		float u1 = pos.x / textureWidth;
		float v1 = pos.y / textureHeight;
		float u2 = (pos.x + size.x) / textureWidth;
		float v2 = (pos.y + size.y) / textureHeight;

		float vertices[] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, u1, v1,
			 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, u2, v1,
			-0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f, u1, v2,
			 0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f, u2, v2
		};

		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		m_RectScale = { u2 - u1,v2 - v1 };
	}

	void Sprite::setRepeat(glm::uvec2 size)
	{
		m_RepeatScale = size;
	}

	void Sprite::setBorderVisiable(bool visible)
	{
		if (visible) {
			setUpBorder();
		}
		else {
			glDeleteVertexArrays(1, &m_BorderVAO);
			glDeleteBuffers(1, &m_BorderVBO);
			glDeleteBuffers(1, &m_BorderEBO);
			delete m_BorderShader;
			m_BorderShader = nullptr;
		}
		m_BorderWidthPixels = 5.f;
		m_BorderColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		m_ShowBorder = visible;
	}

	void Sprite::setAvailable(bool available)
	{
		m_available = available;
	}

	void Sprite::setUpBorder()
	{
		float borderVertices[] = {
			-0.5f, -0.5f, 0.0f,  // 左下
			0.5f, -0.5f, 0.0f,  // 右下
			0.5f,  0.5f, 0.0f,  // 右上
			-0.5f,  0.5f, 0.0f   // 左上
		};
		//unsigned int borderIndices[] = { 0, 1, 2, 1, 2, 3 };

		// 线条索引（绘制四条边）
		unsigned int borderIndices[] = {
			0, 1, // 下边
			1, 2, // 右边
			2, 3, // 上边
			3, 0  // 左边
		};
		glGenVertexArrays(1, &m_BorderVAO);
		glBindVertexArray(m_BorderVAO);

		glGenBuffers(1, &m_BorderVBO);
		glGenBuffers(1, &m_BorderEBO);

		glBindBuffer(GL_ARRAY_BUFFER, m_BorderVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(borderVertices), borderVertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BorderEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(borderIndices), borderIndices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);

		const std::string borderVShader = R"(
		#version 460 core
		layout(location = 0) in vec3 aPos;
		uniform mat4 transform;
		uniform mat4 projection;
		uniform mat4 view;
		void main() {
			gl_Position = projection * view * transform * vec4(aPos, 1.0);
		}
	)";

		const std::string borderFShader = R"(
		#version 460 core
		out vec4 fragColor;
		uniform vec4 borderColor;
		void main() {
			fragColor = borderColor;
		}
	)";

		m_BorderShader = new Shader(borderVShader, borderFShader);
	}

	void Sprite::draw(float right, float top)
	{
		drawBorder(right, top);
		m_Shader->load();
		m_Texture->bind();
		glm::mat4 projection;
		glm::mat4 view(1.f);

		projection = glm::ortho(0.0f, right, 0.0f, top, -1.0f, 1.0f);
		m_Shader->setMat4("projection", projection);
		m_Shader->setMat4("view", view);
		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, m_Position);
		transform = glm::translate(transform, glm::vec3(m_Origin, 0.0f));

		transform = glm::rotate(transform, glm::radians(m_Rotation), glm::vec3(0.0f, 0.0f, 1.0f));
		transform = glm::translate(transform, glm::vec3(-m_Origin, 0.0f));
		glm::vec2 scaledSize = m_Size * m_Scale * m_RectScale * m_RepeatScale;
		transform = glm::scale(transform, glm::vec3(scaledSize.x, scaledSize.y, 1.0f));

		m_Shader->setMat4("transform", transform);
		m_Shader->setVec4("spriteColor", m_Color);
		m_Shader->setVec2("uvScale", m_RepeatScale);

		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		m_Shader->unload();
	}

	void Sprite::drawBorder(float right, float top)
	{
		if (m_ShowBorder) {
			glm::vec2 globalSize = getGlobalSize();
			float borderScaleX = m_BorderWidthPixels / globalSize.x;
			float borderScaleY = m_BorderWidthPixels / globalSize.y;
			glm::mat4 projection = glm::ortho(0.0f, right, 0.0f, top, -1.0f, 1.0f);
			glm::mat4 view(1.0f);
			m_BorderShader->load();
			glBindVertexArray(m_BorderVAO);

			glm::mat4 borderTransform = glm::mat4(1.0f);
			borderTransform = glm::translate(borderTransform, m_Position);
			borderTransform = glm::translate(borderTransform, glm::vec3(m_Origin, 0.0f));
			borderTransform = glm::rotate(borderTransform, glm::radians(m_Rotation), glm::vec3(0.0f, 0.0f, 1.0f));
			borderTransform = glm::translate(borderTransform, glm::vec3(-m_Origin, 0.0f));
			glm::vec2 scaledSize = globalSize * (1.0f + glm::vec2(borderScaleX, borderScaleY));
			borderTransform = glm::scale(borderTransform, glm::vec3(scaledSize.x, scaledSize.y, 1.0f));

			m_BorderShader->setMat4("transform", borderTransform);
			m_BorderShader->setMat4("projection", projection);
			m_BorderShader->setMat4("view", view);
			m_BorderShader->setVec4("borderColor", m_BorderColor);
			glDrawElements(GL_LINES, 8, GL_UNSIGNED_INT, 0);

			//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			m_BorderShader->unload();
		}
	}
}
