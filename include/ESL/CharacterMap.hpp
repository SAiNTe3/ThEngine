#pragma once
#include <iostream>
#include "Sprite.hpp"
#include <unordered_map>

namespace esl
{
	class CharacterMap
	{
		std::unordered_map<char32_t, std::unique_ptr<Sprite>> m_CharacterMap;
		std::unique_ptr<Texture> m_MapTexture;

	protected:

	public:
		CharacterMap() = default;
		CharacterMap(const std::string& filePath);
		~CharacterMap();
		void loadFromFile(const std::string& filePath);
		void loadFromTexture(const esl::Texture& texture);
		void bindCharacter(char32_t ch, glm::vec2 pos, glm::vec2 size);
		void unbindCharacter(char32_t ch);
		Sprite& getCharacterSprite(char32_t ch);
		void clearMap();
	};
};
