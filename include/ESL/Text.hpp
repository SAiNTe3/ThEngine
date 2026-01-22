#pragma once
#include <iostream>
#include "CharacterMap.hpp"
#include <vector>
#include <Shader.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include <memory>
#include <string>
#include <algorithm>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glad/glad.h"
namespace esl
{
	class Text
	{
		CharacterMap* m_CharMap = nullptr;
		std::u32string m_Content;
		std::vector<std::unique_ptr<Sprite>> m_SpriteVector;
		glm::vec2 m_Pos = { 0,0 };
		glm::vec2 m_Scale = { 1,1 };
		glm::uvec4 m_Color = { 255,255,255,255 };
		float m_Angle = 0;
		float m_Space = 0;
		void update();
		friend class Window;
	public:
		Text() = default;
		Text(const std::u32string& text);
		~Text() = default;
		void setText(const std::u32string& text);
		void setText(const std::string& text);
		const std::u32string getText() const;
		void bindMap(CharacterMap& charMap);
		void unbindMap();
		std::u32string operator+(const std::u32string& rv);
		std::u32string operator+(int rv);
		Text& operator+=(const std::u32string& rv);
		Text& operator+=(int rv);
		const char32_t operator[](size_t index);
		void setPosition(glm::vec2 pos);
		void setScale(glm::vec2 scale);
		void setRotation(float angle);
		glm::vec2 getPosition() const;
		glm::vec2 getScale() const;
		float getRotation() const;
		void setCharacterSpace(float space);
		void setColor(glm::vec4 rgba);
		void setColor(glm::uvec4 rgba);
		void setColor(size_t index, glm::vec4 rgba);
		void setColor(size_t index, glm::uvec4 rgba);
		enum class HorizontalAlign
		{
			Left,
			Center,
			Right
		}m_HorizontalAlign=HorizontalAlign::Left;
		void setHorizontalAlign(HorizontalAlign align);
	};
}
