//
// Created by koerriva on 2021/12/20.
//

#ifndef WX_WINDOW_H
#define WX_WINDOW_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string>
#include <glm/vec2.hpp>

namespace wx {
    using namespace std;

    class Window
    {
    private:
        int width = 640;
        int height = 480;
        float aspect = 4.0/3.0;
        bool vsync = true;
        string title;
        bool closed = false;

        GLFWwindow* glfwWindow = nullptr;

        float widthScale=1.0f,heightScale=1.0f;

        double mLastX = width/2,mLastY = height/2,mXOffset,mYOffset;



    public:
        Window(string title,int width,int height,bool vsync);
        ~Window();

        void Init();
        void Update();

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

        [[nodiscard]] int GetFrameBufferWidth() const {
            return width*int(widthScale);
        }

        [[nodiscard]] int GetHeight() const {
            return height;
        }

        [[nodiscard]] int GetFrameBufferHeight() const {
            return height*int(heightScale);
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

        [[nodiscard]] double GetMouseXOffset() const{
            return mXOffset;
        }

        [[nodiscard]] double GetMouseYOffset() const{
            return mYOffset;
        }
    };
}
#endif //WX_WINDOW_H
