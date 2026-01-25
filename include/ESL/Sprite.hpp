#pragma once
#include<iostream>
#include"Texture.hpp"
#include"Shader.hpp"
#include"Render.hpp"
namespace esl
{
	typedef unsigned int GLuint;
	class Sprite : public Renderable
	{
	private:
		
	protected:
		float m_Rotation = 0;
		glm::vec2 m_Origin = glm::vec2(0.f, 0.f);
		Texture* m_Texture = nullptr;
		Shader* m_Shader = nullptr;
		glm::vec3 m_Position = glm::vec3(0.f, 0.f, 0.f);
		glm::vec2 m_Size = glm::vec2(0.f, 0.f);
		glm::vec4 m_Color = glm::vec4(1.f, 1.f, 1.f, 1.f);
		uint m_VAO = 0;
		uint m_VBO = 0;
		uint m_EBO = 0;
		glm::vec2 m_Scale = glm::vec2(1.f, 1.f);
		glm::vec2 m_RectScale = glm::vec2(1.f, 1.f);
		glm::vec2 m_RepeatScale = glm::vec2(1.f, 1.f);
		//border
		glm::vec4 m_BorderColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		float m_BorderWidthPixels = 5.f;
		bool m_ShowBorder = false;
		GLuint m_BorderVAO, m_BorderVBO, m_BorderEBO;
		Shader* m_BorderShader;
		void setUpBorder();
		//给ProgressSprite提供接口
		void bindTexture(){ m_Texture->bind(); }
		bool m_available = true;
	public:
		Sprite();
		Sprite(Texture* texture);
		void setup();
		void setPosition(glm::vec2 pos);
		void setTexture(Texture* texture);
		void setRotation(float angle);
		void setColor(glm::vec4 rgba_float);
		void setColor(glm::uvec4 rgba_int);
		void setScale(glm::vec2 scale);
		void setAlpha(float alpha) { m_Color.a = alpha; }
		void setOrigin(glm::vec2 pos);
		void setTextureRect(glm::vec2 pos, glm::vec2 size);
		void setTextureRectFlip(glm::vec2 pos, glm::vec2 size);
		void setRepeat(glm::uvec2 size);
		void setBorderVisiable(bool visible);
		void setAvailable(bool available);
		
		Texture* getTexture() const;
		glm::vec2 getPosition() const;
		float getRotation() const;
		glm::vec2 getScale() const;
		glm::vec4 getColor() const;
		glm::vec2 getOrigin() const;
		glm::vec2 getGlobalSize();
		glm::vec2 getLocalSize();
		bool getAvailable() const;
		void move(glm::vec2 distance);
	protected:
		virtual void draw(float right, float top)override;
		void drawBorder(float right, float top);
		friend class Window;
	};
}
