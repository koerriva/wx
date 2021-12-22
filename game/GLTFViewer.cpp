//
// Created by koerriva on 2021/12/21.
//

#include "GLTFViewer.h"
namespace wx {
    GLTFViewer::GLTFViewer() {
        renderer = new Renderer();
    }

    GLTFViewer::~GLTFViewer() {

    }

    void GLTFViewer::Init() {
        WX_INFO("GLFTViewer Init");
        renderer->Init();
        this->models = Renderer::LoadModelFromGLTF("model\\Scene.gltf");
        auto sets = Renderer::LoadModelFromGLTF("model\\CesiumDrone.glb");
        for (auto& m:sets) {
            this->models.push_back(m);
        }

        camera = new Camera({0.0,2.0,10.0});
    }

    void GLTFViewer::Input(Window *window) {
        if(window->GetKeyPressed(F1)){
            renderer->SetShaderMode();
        }
        if(window->GetKeyPressed(F2)){
            renderer->SetWireframeMode();
        }
    }

    void GLTFViewer::Update(float elapsedTime) {

    }

    void GLTFViewer::Render(Window *window, float elapsedTime) {
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
        glViewport(0,0,window->GetFrameBufferWidth(),window->GetFrameBufferHeight());

        renderer->Render(window,camera,this->models);
    }

    void GLTFViewer::Cleanup() {
        delete camera;
        delete renderer;
    }
}

int main(int argc,char** argv) {
    system("chcp 65001");

    wx::Log::Init();
    WX_INFO("我的游戏引擎 0.2");
    wx::GLTFViewer game;
    wx::GameEngine engine("我的游戏引擎 0.2.0", 1280, 720, true, &game);
    engine.Run();
}