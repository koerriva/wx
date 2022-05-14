//
// Created by koerriva on 2022/1/16.
//
#include "systems.h"
#include "menu_loader.h"
#include "assets.h"
#define NK_INCLUDE_FIXED_TYPES
#include "nuklear.h"

namespace wx {
    NKMenu load_menu(const char* filename){
        NKMenu nkMenu;
        nkMenu.res = AssetsLoader::LoadText(filename,&nkMenu.resLen);

        using namespace tinyxml2;
        XMLDocument doc;
        doc.Parse(nkMenu.res.c_str(),nkMenu.resLen);

        auto root = doc.RootElement();
        for (XMLElement* pe = root->FirstChildElement("data-bind");pe;pe=pe->NextSiblingElement("data-bind")) {
            for(XMLElement* e = pe->FirstChildElement("data");e;e=e->NextSiblingElement("data")){
                string id = e->Attribute("id");
                string type = e->Attribute("type");

                if(type=="int"){
                    nkMenu.data[id] = e->IntAttribute("val");
                }

                if(type=="float"){
                    nkMenu.data[id] = e->FloatAttribute("val");
                }

                if(type=="float3"){
                    string val(e->Attribute("val"));
                    vec3 buff;
                    string d;
                    int i=0;
                    for (auto c : val) {
                        if(c == ','){
                            buff[i++] = std::stof(d);
                            d = "";
                        }else{
                            d = d + c;
                        }
                    }
                    nkMenu.data[id] = buff;
                }

                if(type=="float4"){
                    string val(e->Attribute("val"));
                    vec4 buff;
                    string d;
                    int i=0;
                    for (auto c : val) {
                        if(c == ','){
                            buff[i++] = std::stof(d);
                            d = "";
                        }else{
                            d = d + c;
                        }
                    }
                    nkMenu.data[id] = buff;
                }
            }
        }

        return nkMenu;
    }

    void render_menu(void* nkCtx, NKMenu& nkMenu){
        auto ctx = (struct nk_context*)nkCtx;

        using namespace tinyxml2;
        XMLDocument doc;
        doc.Parse(nkMenu.res.c_str(),nkMenu.resLen);

        auto root = doc.RootElement();
        for (XMLElement* pe = root->FirstChildElement("window");pe;pe=pe->NextSiblingElement("window")){
            auto title = pe->Attribute("title");
            auto x = pe->FloatAttribute("x");
            auto y = pe->FloatAttribute("y");
            auto width = pe->FloatAttribute("width");
            auto height = pe->FloatAttribute("height");

            if(nk_begin(ctx,title, nk_rect(x,y,width,height),NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
                                                             NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)){

                for (XMLElement* e = pe->FirstChildElement();e;e=e->NextSiblingElement()) {
                    string element = e->Name();
//                    WX_CORE_TRACE("e : {}", element);
                    if(element=="layout"){
                        nk_layout_row_dynamic(ctx,e->FloatAttribute("height"),e->IntAttribute("cols"));
                    }
                    if(element=="property"){
                        string type = e->Attribute("type");
                        auto name = e->Attribute("name");

                        NKData& data = nkMenu.data[e->Attribute("data-bind")];

                        if(type=="int"){
                            int min = e->IntAttribute("min");
                            int max = e->IntAttribute("max");
                            int step = e->IntAttribute("step");
                            float resolution = e->FloatAttribute("resolution");

                            data = nk_propertyi(ctx, name, min, std::get<int>(data), max, step, resolution);
                        }
                        if(type=="float"){
                            float min = e->FloatAttribute("min");
                            float max = e->FloatAttribute("max");
                            float step = e->FloatAttribute("step");
                            float resolution = e->FloatAttribute("resolution");

                            data = nk_propertyf(ctx, name, min, std::get<float>(data), max, step, resolution);
                        }
                    }
                    if(element=="label"){
                        auto text = e->Attribute("text");
                        string align = e->Attribute("align");
                        nk_text_alignment alignment;
                        if(align=="left"){
                            alignment = NK_TEXT_LEFT;
                        }
                        if(align=="center"){
                            alignment = NK_TEXT_CENTERED;
                        }
                        if(align=="right"){
                            alignment = NK_TEXT_RIGHT;
                        }
                        nk_label(ctx, text, alignment);
                    }
                    if(element=="combocolor"){
                        NKData& data = nkMenu.data[e->Attribute("data-bind")];
                        vec4 color = std::get<vec4>(data);
                        nk_colorf bind_color{color.r,color.g,color.b,color.a};

                        if(nk_combo_begin_color(ctx, nk_rgba_cf(bind_color), nk_vec2(nk_widget_width(ctx),400))) {
                            for (XMLElement *ce = e->FirstChildElement(); ce; ce = ce->NextSiblingElement()) {
                                string _element = ce->Name();
//                                WX_CORE_TRACE("e : {}", element);
                                if (_element == "layout") {
                                    nk_layout_row_dynamic(ctx, ce->FloatAttribute("height"), ce->IntAttribute("cols"));
                                }
                                if (_element == "colorpicker") {
                                    bind_color = nk_color_picker(ctx, bind_color, NK_RGBA);

                                    nk_layout_row_dynamic(ctx, 25, 1);
                                    bind_color.r = nk_propertyf(ctx, "#R:", 0, bind_color.r, 1.0f, 0.01f, 0.005f);
                                    bind_color.g = nk_propertyf(ctx, "#G:", 0, bind_color.g, 1.0f, 0.01f, 0.005f);
                                    bind_color.b = nk_propertyf(ctx, "#B:", 0, bind_color.b, 1.0f, 0.01f, 0.005f);
                                    bind_color.a = nk_propertyf(ctx, "#A:", 0, bind_color.a, 1.0f, 0.01f, 0.005f);
                                }
                            }

                            color.r = bind_color.r;
                            color.g = bind_color.g;
                            color.b = bind_color.b;
                            color.a = bind_color.a;
                            data = color;

                            nk_combo_end(ctx);
                        }
                    }
                    if(element=="selecti"){
                        NKData& data = nkMenu.data[e->Attribute("data-bind")];
                        int value = std::get<int>(data);

                        for (XMLElement* ce = e->FirstChildElement("option");ce;ce=ce->NextSiblingElement("option")){
                            auto name = ce->Attribute("name");
                            int val = ce->IntAttribute("val");
                            if(nk_option_label(ctx,name,value==val)){
                                value = val;
                            }
                        }

                        data = value;
                    }
                    if(element=="spacer"){
                        nk_spacer(ctx);
                    }
                }
            }
            nk_end(ctx);
        }
    }

    void render_nuklear_phase(level* level,float delta){
        WX_CORE_TRACE("render nuklear");
    }
}
