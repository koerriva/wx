//
// Created by koerriva on 2021/12/20.
//

#include "font.h"

#include "log.h"
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace wx {
    using namespace glm;
    struct Character{
        bool cached=false;
        GLuint texture;
        ivec2 size;
        ivec2 bearing;
        GLuint advance;
    };

    std::unordered_map<wchar_t,Character> chars;

    FT_Library ft;
    FT_Face fc;

    Font* Font::_instance = nullptr;

    void Font::_init() {
        if(FT_Init_FreeType(&ft)){
            WX_CORE_ERROR("Can't Init FreeType Library");
        }
        if(FT_New_Face(ft,fontpath,0,&fc)){
            WX_CORE_ERROR("Can't Load Font [{}]",fontpath);
        }
        FT_Set_Pixel_Sizes(fc,0,PIXEL_SIZE);

        glPixelStorei(GL_UNPACK_ALIGNMENT,1);//禁用字节对齐
        for (wchar_t c = 0; c < 128; ++c) {
            if(FT_Load_Char(fc,c,FT_LOAD_RENDER)){
                WX_CORE_ERROR("Can't Load char");
                continue;
            }
            _genTexture(c);
        }
//            for (wchar_t c = 0x4e00; c < 0x9fa5; ++c) {
//                if(FT_Load_Char(fc,c,FT_LOAD_RENDER)){
//                    Logger::Error("Can't Load char");
//                    continue;
//                }
//                _GenTexture(c);
//            }
        glBindTexture(GL_TEXTURE_2D,0);
    }

    void Font::_genTexture(wchar_t c) {
        GLuint texture;
        glGenTextures(1,&texture);
        glBindTexture(GL_TEXTURE_2D,texture);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RED
                ,fc->glyph->bitmap.width,fc->glyph->bitmap.rows
                ,0,GL_RED,GL_UNSIGNED_BYTE
                ,fc->glyph->bitmap.buffer);
        // 设置纹理选项
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        Character character{true,texture
                ,ivec2{fc->glyph->bitmap.width,fc->glyph->bitmap.rows}
                ,ivec2{fc->glyph->bitmap_left,fc->glyph->bitmap_top}
                ,static_cast<GLuint>(fc->glyph->advance.x)};
        chars[c] = character;
    }

    void Font::_cleanup() {
        chars.clear();
    }

    Character Font::GetChar(wchar_t c) {
        auto& ch = chars[c];
        if(!ch.cached){
            if(FT_Load_Char(fc,c,FT_LOAD_RENDER)){
                WX_CORE_ERROR("Can't Load char");
            }
            _instance->_genTexture(c);
        }
        return chars[c];
    }

    void Font::Cleanup() {
        _instance->_cleanup();
        FT_Done_Face(fc);
        FT_Done_FreeType(ft);
    }

    void Font::Init() {
        _instance = new Font("data/font/NotoSansSC-Regular.otf");
        _instance->_init();
    }
}