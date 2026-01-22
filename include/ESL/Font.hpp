#pragma once
#include"glad/glad.h"
#include"GLFW/glfw3.h"
#include <ft2build.h>
#include FT_FREETYPE_H  
#include <Shader.hpp>

namespace esl {

    class Font
    {
        static FT_Library ft;
		
    public:
        FT_Face face;
        ~Font();
        static void init();
        static void destory();
        void loadFromFile(const std::string& font_path);
        void setFontSize(int size);
        friend class TextM;
        friend class TextW;
    };
}