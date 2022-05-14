//
// Created by koerriva on 2022/5/13.
//

#ifndef WX_MENU_LOADER_H
#define WX_MENU_LOADER_H

#include <tinyxml2.h>

namespace wx {
    NKMenu load_menu(const char* filename);
    void render_menu(void* ctx,NKMenu& nkMenu);
}

#endif //WX_MENU_LOADER_H
