#include "Font.hpp"

namespace esl {
    FT_Library Font::ft;
    void Font::setFontSize(int size)
    {
        FT_Set_Pixel_Sizes(face, 0, size);
    }
    Font::~Font()
    {
        FT_Done_Face(face);
    }
    void Font::init()
    {
        FT_Init_FreeType(&ft);
    }
    void Font::destory()
    {
        FT_Done_FreeType(ft);
    }
    void Font::loadFromFile(const std::string& font_path)
    {
        FT_New_Face(ft, font_path.c_str(), 0, &face);
        FT_Set_Pixel_Sizes(face, 0, 48);
    }
}