//
// Created by koerriva on 2021/12/20.
//

#ifndef WX_WINDOW_H
#define WX_WINDOW_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string>

namespace wx {
    using namespace std;

    enum KeyCode {
        ESC = GLFW_KEY_ESCAPE,ENTER,TAB,
        RIGHT=GLFW_KEY_RIGHT,LEFT,DOWN,UP,
        F1 = GLFW_KEY_F1,
        F2,F3,F4,F5,F6,F7,F8,F9,F10,F11,F12,
        Num0 = GLFW_KEY_0,Num1,Num2,Num3,Num4,Num5,Num6,Num7,Num8,Num9,
        A=GLFW_KEY_A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z
    };

    enum MouseCode {
        M_LEFT = GLFW_MOUSE_BUTTON_LEFT,
        M_MID = GLFW_MOUSE_BUTTON_MIDDLE,
        M_RIGHT = GLFW_MOUSE_BUTTON_RIGHT
    };

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

        int last_key_state[GLFW_KEY_LAST+1] = {0};
        int curr_key_state[GLFW_KEY_LAST+1] = {0};

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

        //Input
        [[nodiscard]] bool GetKeyPressed(int key) const;
        bool GetKeyDown(int key) const;

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

        [[nodiscard]] bool GetMouseButtonPressed(int btn) const{
            int state = glfwGetMouseButton(glfwWindow,btn);
            return state==GLFW_PRESS;
        }
    };
}
#endif //WX_WINDOW_H
