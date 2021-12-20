//
// Created by koerriva on 2021/12/20.
//

#ifndef WX_FONT_H
#define WX_FONT_H

#include <unordered_map>

namespace wx {
    using namespace std;

    struct Character;
    class Font{
    private:
        const char* fontpath;
        explicit Font(const char* fontpath){
            this->fontpath = fontpath;
        }
        ~Font()= default;
        void _init();
        void _genTexture(wchar_t c);
        void _cleanup();
        static Font* _instance;
    public:
        static const int PIXEL_SIZE = 16;
        static void Init();
        static Character GetChar(wchar_t c);
        static void Cleanup();
    };
}
#endif //WX_FONT_H
