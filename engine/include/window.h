//
// Created by koerriva on 2021/12/20.
//

#ifndef WX_WINDOW_H
#define WX_WINDOW_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string>
#include <glm/vec2.hpp>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_KEYSTATE_BASED_INPUT
#include "nuklear.h"
#include "nuklear_glfw_gl3.h"

namespace wx {
    using namespace std;

    class Window
    {
    private:
        int width = 640;
        int height = 480;
        float aspect = 4.0/3.0;
        bool vsync = true;
        string title = "Meta Engine";
        bool closed = false;
        GLFWwindow* glfwWindow = nullptr;
        float widthScale=1.0f,heightScale=1.0f;

        //nuklear
        struct nk_glfw nkGlfw{nullptr};
        struct nk_context* nkContext;
    public:
        Window();
        Window(string title,int width,int height,bool vsync);
        ~Window();

        void Init();
        void Update();

        void SetWindowTitle(const char* name);
        void SetWindowSize(int width,int height);
        void SetVSync(bool vSync);
        void ShowWindow(bool visible);
        void SetToCenter();

        void* GetNKGlfw(){
            return &nkGlfw;
        }

        void* GetNKCtx(){
            return nkContext;
        }

        [[nodiscard]] bool Closed() const {
            return closed;
        }

        void Close(){
            glfwSetWindowShouldClose(glfwWindow,true);
        }

        void ShowCursor(bool enable){
            if(enable){
                glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }else{
                glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
        }

        void SetCursor(unsigned char* icon_data,int w,int h,int xhot,int yhot){
            GLFWimage img;
            img.width = w;img.height = h;
            img.pixels = icon_data;
            auto cursor = glfwCreateCursor(&img,xhot,yhot);
            glfwSetCursor(glfwWindow,cursor);
        }

        void Cleanup();

        int GetKeyState(int key);
        int GetMouseState(int btn);
        void GetCursorPos(double * x,double * y);

        [[nodiscard]] int GetWidth() const {
            return width;
        }

        [[nodiscard]] int GetHeight() const {
            return height;
        }

        [[nodiscard]] static double GetTimeInSecond() {
            return glfwGetTime();
        }

        [[nodiscard]] float GetAspect() const{
            return aspect;
        }

        [[nodiscard]] bool VSynced() const {
            return vsync;
        }
    };
}
#endif //WX_WINDOW_H
