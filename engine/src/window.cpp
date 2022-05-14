//
// Created by koerriva on 2021/12/20.
//
#include <iostream>
#include <glad/glad.h>

#include "window.h"
#include "log.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include "systems.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
//#define NK_INCLUDE_FONT_BAKING
//#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include "nuklear.h"
#define NK_GLFW_GL3_IMPLEMENTATION
#include "nuklear_glfw_gl3.h"

namespace wx {
    void window_update_system(level* level,float delta){
        auto window = level_get_share_resource<Window>(level);
        window->Update();
    }

    Window::Window() {}
    Window::Window(string title,int width,int height,bool vsync)
    {
        this->height=height;
        this->width=width;
        this->vsync=vsync;
        this->title = std::move(title);
        this->aspect = float(width)/float(height);
    }

    Window::~Window() = default;

    void Window::Init(){
        if(glfwInit()){
            glfwWindowHint(GLFW_SCALE_TO_MONITOR,GLFW_TRUE);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_SAMPLES,4);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
            glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
            glfwWindowHint(GLFW_VISIBLE,GL_FALSE);
#ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
            glfwWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
            if(!glfwWindow){
                glfwTerminate();
                WX_CORE_CRITICAL("Create GLFW Window Fail!");
                exit(-1);
            }

            //
            int w,h;
            glfwGetWindowSize(glfwWindow,&w,&h);
            WX_CORE_INFO("Window Size ({},{})",w,h);
            glfwGetFramebufferSize(glfwWindow,&w,&h);
            WX_CORE_INFO("FrameBuffer Size Size ({},{})",w,h);
            glfwGetWindowContentScale(glfwWindow,&widthScale,&heightScale);
            WX_CORE_INFO("Window Scale ({},{})",widthScale,heightScale);

            glfwSetWindowSize(glfwWindow,width,height);
            //set window to center
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            if(mode){
                int mW=mode->width,mH=mode->height;
                WX_CORE_INFO("Monitor Size {},{}",mW,mH);
                glfwSetWindowPos(glfwWindow,(mW-width)/2,(mH-height)/2);
            }

            glfwMakeContextCurrent(glfwWindow);

            if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
                WX_CORE_CRITICAL("Load OpenGL Fail!");
                exit(-1);
            }

            WX_CORE_INFO("OpenGL Version {}.{}",GLVersion.major,GLVersion.minor);
            if(vsync){
                glfwSwapInterval(1);
            }

            //nuklear
            nkContext = nk_glfw3_init(&nkGlfw,glfwWindow,NK_GLFW3_INSTALL_CALLBACKS);
            /* Load Fonts: if none of these are loaded a default font will be used  */
            /* Load Cursor: if you uncomment cursor loading please hide the cursor */
            {
                NK_STORAGE const nk_rune ranges[] = {
                        0x0020, 0x00FF,
                        0xFF01, 0xFF0F,
                        0x4e00, 0x9FAF,
                        0
                };

                struct nk_font_config config = nk_font_config(0);
                config.oversample_h = 1;
                config.oversample_v = 1;
                config.range = ranges;

                struct nk_font_atlas *atlas;
                nk_glfw3_font_stash_begin(&nkGlfw,&atlas);
                struct nk_font *notosans = nk_font_atlas_add_from_file(atlas, "data/font/HanSans.otf", 18, &config);
                nk_glfw3_font_stash_end(&nkGlfw);
//                nk_style_load_all_cursors(nkContext, atlas->cursors);
                nk_style_set_font(nkContext, &notosans->handle);
            }

            /*set_style(ctx, THEME_WHITE);*/
            /*set_style(ctx, THEME_RED);*/
            /*set_style(ctx, THEME_DARK);*/
        }
    }

    void Window::Update(){
        glfwPollEvents();
        closed = glfwWindowShouldClose(glfwWindow)==1;
        glfwSwapBuffers(glfwWindow);
    }

    void Window::Cleanup(){
        glfwTerminate();
    }

    int Window::GetKeyState(int key) {
        return glfwGetKey(glfwWindow,key);
    }

    int Window::GetMouseState(int btn){
        return glfwGetMouseButton(glfwWindow,btn);
    }

    void Window::GetCursorPos(double * x,double * y){
        glfwGetCursorPos(glfwWindow,x,y);
    }

    void Window::SetWindowTitle(const char *name) {
        this->title = name;
        glfwSetWindowTitle(glfwWindow,name);
    }

    void Window::SetWindowSize(int width, int height) {
        this->width = width;
        this->height = height;
        this->aspect = float(this->width)/float(this->height);
        glfwSetWindowSize(glfwWindow,width,height);
    }

    void Window::SetVSync(bool vSync) {
        this->vsync = vSync;
        glfwSwapInterval(vSync?1:0);
    }

    void Window::ShowWindow(bool visible) {
        if(visible){
            glfwShowWindow(glfwWindow);
        }else{
            glfwHideWindow(glfwWindow);
        }
    }

    void Window::SetToCenter() {
        //set window to center
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        if(mode){
            int mW=mode->width,mH=mode->height;
            WX_CORE_INFO("Monitor Size {},{}",mW,mH);
            glfwSetWindowPos(glfwWindow,(mW-width)/2,(mH-height)/2);
        }
    }
}