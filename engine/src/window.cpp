//
// Created by koerriva on 2021/12/20.
//
#include <iostream>
#include <glad/glad.h>

#include "window.h"
#include "log.h"

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
                std::cerr << "Create GLFW Window Fail!"<<std::endl;
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
                std::cerr << "Load OpenGL Fail!" << std::endl;
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

            glViewport(0,0,width*int(widthScale),height*int(heightScale));
        }

    }

    void Window::Update(){
        closed = glfwWindowShouldClose(glfwWindow)==1;
        glfwGetWindowSize(glfwWindow,&width,&height);
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
    }

    void Window::Cleanup(){
        glfwTerminate();
    }

    bool Window::GetKeyPressed(int key) const{
        return glfwGetKey(glfwWindow,key)==GLFW_PRESS;
    }
}