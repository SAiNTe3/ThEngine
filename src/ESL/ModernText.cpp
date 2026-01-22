#include "ModernText.hpp"


namespace esl {
    template<class T>
    ModernText<T>::ModernText() {
        std::string vertex, fragment;
        vertex = {
            "#version 460 core\n"
            "layout(location = 0) in vec4 vertex;\n"
            "out vec2 TexCoords;\n"
            "uniform mat4 projection;\n"
            "void main(){\n"
            "gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
            "TexCoords = vertex.zw;\n}\0"
        };
        fragment = {
            "#version 460 core\n"
            "in vec2 TexCoords;\n"
            "out vec4 color;\n"
            "uniform sampler2D text;\n"
            "uniform vec4 textColor;\n"
            "void main(){\n"
            "vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"
            "color = textColor * sampled;\n"
            "}\0"
        };
        shader = new Shader(vertex, fragment);
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    template<class T>
    ModernText<T>::~ModernText()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        for (auto& c : charMap) {
            glDeleteTextures(1, &c.second.TextureID);
        }
    }
    template<class T>
    void ModernText<T>::updateMap()
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        for (size_t i = 0; i < text.size(); i++) {
            if (!charMap.count(text[i])) {
                FT_Load_Char(font->face, text[i], FT_LOAD_RENDER);
                GLuint texture;
                glGenTextures(1, &texture);
                glBindTexture(GL_TEXTURE_2D, texture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, font->face->glyph->bitmap.width, font->face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, font->face->glyph->bitmap.buffer);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                Character character = {
                    texture,
                    glm::ivec2(font->face->glyph->bitmap.width, font->face->glyph->bitmap.rows),
                    glm::ivec2(font->face->glyph->bitmap_left, font->face->glyph->bitmap_top),
                    font->face->glyph->advance.x
                };
                charMap.insert(std::pair<GLchar, Character>(text[i], character));
            }
        }
    }
    template<class T>
    void ModernText<T>::setFont(Font& font) {
        this->font = &font;
        if (!text.empty()) {
            charMap.clear();
            updateMap();
        }
    }
    template<class T>
    void ModernText<T>::setSize(int size)
    {
        this->size = size;
        if (font) {
            font->setFontSize(size);
            // 重新生成所有字符纹理
            charMap.clear();
            if (!text.empty()) {
                updateMap();
            }
        }
    }
    template<class T>
    void ModernText<T>::setText(const T& text) {
        this->text = text;
        if (font) {
            updateMap();
        }
    }
    template<class T>
    void ModernText<T>::setColor(glm::vec3 color) {
        this->color = glm::vec4(color, this->color.a);
    }
    template<class T>
    void ModernText<T>::setColor(glm::vec4 color)
    {
        this->color = color;
    }
    template<class T>
    void ModernText<T>::setPosition(glm::vec2 pos) {
        this->pos = pos;
    }
    template<class T>
    void ModernText<T>::setScale(glm::vec2 scale) {
        this->scale = scale;
    }
    template<class T>
    void ModernText<T>::draw(float right, float top) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glm::mat4 projection = glm::ortho(0.0f, right, 0.0f, top);
        shader->load();
        shader->setMat4("projection", projection);
        shader->setVec4("textColor", color);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);
        glm::vec2 text_pos = pos;
        T::const_iterator c;
        for (c = text.begin(); c != text.end(); c++) {
            Character ch = charMap[*c];

            GLfloat xpos = text_pos.x + ch.Bearing.x * scale.x;
            GLfloat ypos = text_pos.y - (ch.Size.y - ch.Bearing.y) * scale.y;

            GLfloat w = ch.Size.x * scale.x;
            GLfloat h = ch.Size.y * scale.y;

            // 更新VBO
            GLfloat vertices[6][4] = {
                { xpos,     ypos + h,   0.0, 0.0 },
                { xpos,     ypos,       0.0, 1.0 },
                { xpos + w, ypos,       1.0, 1.0 },

                { xpos,     ypos + h,   0.0, 0.0 },
                { xpos + w, ypos,       1.0, 1.0 },
                { xpos + w, ypos + h,   1.0, 0.0 }
            };

            // 渲染字符纹理
            glBindTexture(GL_TEXTURE_2D, ch.TextureID);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // 更新位置到下一个字符
            text_pos.x += (ch.Advance >> 6) * scale.x; // 位偏移6个单位来获取1/64像素
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    template class ModernText<std::string>;
    template class ModernText<std::wstring>;
}