#pragma once
#include <iostream>
#include <Font.hpp>
#include <Render.hpp>
#include <map>

namespace esl {

    template<class T>
    class ModernText : public Renderable
    {
        GLuint VAO, VBO;
        struct Character {
            GLuint TextureID;
            glm::ivec2 Size;
            glm::ivec2 Bearing;
            GLuint Advance;
        };
        Shader* shader = nullptr;
        T text;
        std::map<GLchar, Character> charMap;
        Font* font = nullptr;
        glm::vec4 color{ 0,0,0,1 };
        glm::vec2 pos{ 0,0 };
        glm::vec2 scale{ 1,1 };
        int size = 0;
    public:
        ModernText();
        ~ModernText();
        void updateMap();
        void setFont(Font& font);
        void setSize(int size);
		int getSize() { return size; }
        void setText(const T& text);
        void setColor(glm::vec3 color);
        void setColor(glm::vec4 color);
        void setPosition(glm::vec2 pos);
        void setScale(glm::vec2 scale);
		glm::vec2 getScale() { return scale; }
		size_t getLength() { return text.length(); }
        void draw(float right, float top);
    };
    using SText = ModernText<std::string>;
    using WText = ModernText<std::wstring>;
}