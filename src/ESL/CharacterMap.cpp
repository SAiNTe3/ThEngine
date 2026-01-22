#include "CharacterMap.hpp"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

namespace esl
{
	void CharacterMap::loadFromFile(const std::string& filePath)
	{
		m_MapTexture = std::make_unique<Texture>(filePath.c_str());
	}

	void CharacterMap::loadFromTexture(const esl::Texture& texture)
	{
		m_MapTexture = std::make_unique<Texture>(texture);
	}


	CharacterMap::CharacterMap(const std::string& filePath)
	{
		this->loadFromFile(filePath);
	}

	CharacterMap::~CharacterMap()
	{
		for (auto& pair : m_CharacterMap)
		{
			pair.second.release();
		}
		this->clearMap();
	}

	void CharacterMap::bindCharacter(char32_t ch, glm::vec2 pos, glm::vec2 size)
	{
		m_CharacterMap[ch] = std::make_unique<Sprite>(m_MapTexture.get());
		m_CharacterMap[ch]->setTextureRect(pos, size);
	}

	void CharacterMap::unbindCharacter(char32_t ch)
	{
		m_CharacterMap[ch] = nullptr;
	}

	Sprite& CharacterMap::getCharacterSprite(char32_t ch)
	{
		return *m_CharacterMap[ch];
	}

	void CharacterMap::clearMap()
	{
		m_CharacterMap.clear();
	}
}

