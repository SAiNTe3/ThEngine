#include "Text.hpp"
#include <locale>
#include <codecvt>
#include <stdexcept>
#include <sstream>
#include <fstream>  // 添加文件流支持
#include "glad/glad.h"
#include "GLFW/glfw3.h"
namespace esl
{

	void Text::update()
	{
		glm::vec2 pos = m_Pos;
		float angle = m_Angle / 180.f * glm::pi<float>();
		glm::vec2 offset = glm::vec2(m_Space * cos(angle), m_Space * sin(angle));
		float totalWidth = 0.0f;
		for (auto& e : m_SpriteVector)
		{
			e->setColor(m_Color);
		}
		int count = static_cast<int>(m_SpriteVector.size());
		switch (m_HorizontalAlign) {
		case HorizontalAlign::Left:
			for (auto& e : m_SpriteVector)
			{
				e->setPosition(pos);
				e->setScale(m_Scale);
				e->setRotation(m_Angle);
				pos += offset;
			}
			break;
		case HorizontalAlign::Center:
			if (count % 2) {
				// 奇数：中间字符放在中心位置
				int centerIndex = count / 2;
				m_SpriteVector[centerIndex]->setPosition(pos);
				m_SpriteVector[centerIndex]->setScale(m_Scale);
				m_SpriteVector[centerIndex]->setRotation(m_Angle);

				// 左侧和右侧字符
				for (int i = 1; i <= count / 2; ++i)
				{
					// 左侧字符
					m_SpriteVector[centerIndex - i]->setPosition(pos - offset * static_cast<float>(i));
					m_SpriteVector[centerIndex - i]->setScale(m_Scale);
					m_SpriteVector[centerIndex - i]->setRotation(m_Angle);
					// 右侧字符
					m_SpriteVector[centerIndex + i]->setPosition(pos + offset * static_cast<float>(i));
					m_SpriteVector[centerIndex + i]->setScale(m_Scale);
					m_SpriteVector[centerIndex + i]->setRotation(m_Angle);
				}
			}
			else {
				// 偶数：在中心点两侧放置字符
				float halfOffset = 0.5f;
				for (int i = 0; i < count / 2; ++i)
				{
					// 左侧字符 (从右到左放置)
					m_SpriteVector[count / 2 - 1 - i]->setPosition(pos - offset * (halfOffset + i));
					m_SpriteVector[count / 2 - 1 - i]->setScale(m_Scale);
					m_SpriteVector[count / 2 - 1 - i]->setRotation(m_Angle);
					// 右侧字符 (从左到右放置)
					m_SpriteVector[count / 2 + i]->setPosition(pos + offset * (halfOffset + i));
					m_SpriteVector[count / 2 + i]->setScale(m_Scale);
					m_SpriteVector[count / 2 + i]->setRotation(m_Angle);
				}
			}
			break;
		case HorizontalAlign::Right:
			// 右对齐：从右往左放置字符
			{
				glm::vec2 rightStartPos = pos - offset * static_cast<float>(count - 1);
				for (int i = 0; i < count; ++i)
				{
					m_SpriteVector[i]->setPosition(rightStartPos + offset * static_cast<float>(i));
					m_SpriteVector[i]->setScale(m_Scale);
					m_SpriteVector[i]->setRotation(m_Angle);
				}
			}
			break;
		default:
			break;
		}

	}

	Text::Text(const std::u32string& text)
	{
		this->m_Content = text;
		m_SpriteVector.clear();
		std::cout << "Text::Text Warning: No charMap bind!" << std::endl;
	}

	void Text::setText(const std::u32string& text)
	{
		m_Content = text;
		m_SpriteVector.clear();
		if (!m_CharMap) throw std::runtime_error("Text::setText Failed! No character map loaded!");

		for (size_t i = 0; i < m_Content.size(); ++i)
		{
			m_SpriteVector.push_back(std::make_unique<Sprite>(m_CharMap->getCharacterSprite(m_Content[i])));
		}
		update();
	}

	void Text::setText(const std::string& text)
	{
		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
		m_Content = converter.from_bytes(text);
		m_SpriteVector.clear();
		if (!m_CharMap) throw std::runtime_error("Text::setText Failed! No character map loaded!");
		for (size_t i = 0; i < m_Content.size(); ++i)
		{
			m_SpriteVector.push_back(std::make_unique<Sprite>(m_CharMap->getCharacterSprite(m_Content[i])));
		}
		update();
	}

	const std::u32string Text::getText() const
	{
		return m_Content;
	}

	void Text::bindMap(CharacterMap& charMap)
	{
		m_CharMap = &charMap;
		if (!m_Content.empty()) {
			for (size_t i = 0; i < m_Content.size(); ++i)
			{
				m_SpriteVector.push_back(std::make_unique<Sprite>(m_CharMap->getCharacterSprite(m_Content[i])));
			}
		}
	}

	void Text::unbindMap()
	{
		m_CharMap = nullptr;
	}

	std::u32string Text::operator+(const std::u32string& rv)
	{
		return m_Content + rv;
	}

	std::u32string Text::operator+(int rv)
	{
		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
		std::u32string str = converter.from_bytes(std::to_string(rv));
		return m_Content + str;
	}

	Text& Text::operator+=(const std::u32string& rv)
	{
		m_Content += rv;
		this->setText(m_Content);
		return *this;
	}

	Text& Text::operator+=(int rv)
	{
		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
		std::u32string str = converter.from_bytes(std::to_string(rv));
		m_Content += str;
		this->setText(m_Content);
		return *this;
	}

	const char32_t Text::operator[](size_t index)
	{
		return m_Content[index];
	}

	void Text::setPosition(glm::vec2 pos)
	{
		m_Pos = pos;
		update();
	}

	void Text::setScale(glm::vec2 scale)
	{
		m_Scale = scale;
		update();
	}

	void Text::setRotation(float angle)
	{
		m_Angle = angle;
		update();
	}

	glm::vec2 Text::getPosition() const
	{
		return m_Pos;
	}

	glm::vec2 Text::getScale() const
	{
		return m_Scale;
	}

	float Text::getRotation() const
	{
		return m_Angle;
	}

	void Text::setCharacterSpace(float space)
	{
		m_Space = space;
		update();
	}

	void Text::setColor(glm::vec4 rgba)
	{
		m_Color = glm::uvec4(rgba * 255.0f);
		for (auto& e : m_SpriteVector)
		{
			e->setColor(rgba);
		}
	}

	void Text::setColor(glm::uvec4 rgba)
	{
		m_Color = rgba;
		for (auto& e : m_SpriteVector)
		{
			e->setColor(rgba);
		}
	}

	void Text::setColor(size_t index, glm::vec4 rgba)
	{
		if (index >= m_SpriteVector.size()) throw std::runtime_error("Text::setColor Failed! Index out of bounds!");
		m_SpriteVector[index]->setColor(rgba);
	}

	void Text::setColor(size_t index, glm::uvec4 rgba)
	{
		if (index >= m_SpriteVector.size()) throw std::runtime_error("Text::setColor Failed! Index out of bounds!");
		m_SpriteVector[index]->setColor(rgba);
	}

	void Text::setHorizontalAlign(HorizontalAlign align)
	{
		m_HorizontalAlign = align;
		update();
	}

} // namespace esl