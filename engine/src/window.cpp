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

namespace wx {
    Window::Window(string title,int width,int height,bool vsync)
    {
        this->height=height;
        this->width=width;
        this->vsync=vsync;
        this->title = std::move(title);
    }

    Window::~Window() = default;

    void Window::Init(){
        if(glfwInit()){
            glfwWindowHint(GLFW_SCALE_TO_MONITOR,GLFW_TRUE);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
            glfwWindowHint(GLFW_SAMPLES,4);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
#ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

            glfwWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
            if(!glfwWindow){
                glfwTerminate();
                WX_CORE_CRITICAL("Create GLFW Window Fail!");
                exit(-1);
            }

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

            int w,h;
            glfwGetWindowSize(glfwWindow,&w,&h);
            WX_CORE_INFO("Window Size ({},{})",w,h);
            glfwGetFramebufferSize(glfwWindow,&w,&h);
            WX_CORE_INFO("FrameBuffer Size Size ({},{})",w,h);
            glfwGetWindowContentScale(glfwWindow,&widthScale,&heightScale);
            WX_CORE_INFO("Window Scale ({},{})",widthScale,heightScale);
        }

    }

    void Window::Update(){
        closed = glfwWindowShouldClose(glfwWindow)==1;
//        glfwGetWindowSize(glfwWindow,&width,&height);
        double mX,mY;
        glfwGetCursorPos(glfwWindow,&mX,&mY);
        mXOffset = mX - mLastX;
        mYOffset = mLastY - mY;
        mLastX = mX;
        mLastY = mY;
        float sensitivity = 0.05f;
        mXOffset *= sensitivity;
        mYOffset *= sensitivity;

        glfwSwapBuffers(glfwWindow);
        glfwPollEvents();

        for (int i = 0; i < GLFW_KEY_LAST + 1; ++i) {
            int key_state = glfwGetKey(glfwWindow,i);
            last_key_state[i] = curr_key_state[i];
            curr_key_state[i] = key_state;
        }
    }

    void Window::Cleanup(){
        glfwTerminate();
    }

    bool Window::GetKeyPressed(int key) const{
        return last_key_state[key]==GLFW_PRESS&&curr_key_state[key]==GLFW_RELEASE;
    }

    bool Window::GetKeyDown(int key) const {
        return curr_key_state[key]==GLFW_PRESS;
    }
}