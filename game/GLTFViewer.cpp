//
// Created by koerriva on 2021/12/21.
//

#include <stb/stb_image.h>
#include "GLTFViewer.h"

namespace wx {
    GLTFViewer::GLTFViewer() {
        renderer = new Renderer();
    }

    GLTFViewer::~GLTFViewer() {

    }

    void GLTFViewer::Init(Window *window) {
        WX_INFO("GLFTViewer Init");
        renderer->Init();
//        auto Scene_sets = Renderer::LoadModelFromGLTF("model\\Scene.gltf");
        auto CesiumDrone_sets = Renderer::LoadModelFromGLTF("model\\CesiumDrone.glb");
//        auto Snake = Renderer::LoadModelFromGLTF("model\\Snake.gltf");
//        auto Sphere = Renderer::LoadModelFromGLTF("model\\Sphere.gltf");
//        for (auto& m:Scene_sets) {
//            this->models.push_back(m);
//        }
        for (auto& m:CesiumDrone_sets) {
            m.transform.position += vec3(4,5,1);
            this->models.push_back(m);
        }
//        for (auto& m : Snake) {
//            this->models.push_back(m);
//        }
//        for (auto& m : Sphere) {
//            this->models.push_back(m);
//        }

        camera = new Camera({0.0,2.0,10.0});

        int len;
        const unsigned char * img_buffer = AssetsLoader::LoadTexture("cursor-32-2.png",&len);
        int w,h,comp;
        auto img = stbi_load_from_memory(img_buffer,len,&w,&h,&comp,0);
//        window->SetCursor(img, w, h, 9, 3);
        window->SetCursor(img, w, h, 7, 1);

//        light_t pointLight;
//        pointLight.type = point;
//        pointLight.color = vec3{1.0f};
//        pointLight.position = vec3{0,5,-1};
//        pointLight.intensity = 10;
//        pointLight.attenuation = {0.0,0.02,1.1};
//        lights.push_back(pointLight);
//
//        light_t dirLight;
//        dirLight.type = directional;
//        dirLight.color = vec3{1.0f};
//        dirLight.direction = vec3{0,-1,0};
//        dirLight.intensity = 10;
//        lights.push_back(dirLight);

        light_t spotLight;
        spotLight.type = spot;
        spotLight.color = vec3{1.0f,1.0,1.0};
        spotLight.position = vec3{0.f,10.,0.};
        spotLight.direction = vec3{0.f,-1.f,0.};
        spotLight.cutoff = glm::cos(radians(60.0f));
        spotLight.intensity = 10;
        lights.push_back(spotLight);
    }

    void GLTFViewer::Input(Window *window) {
        if(window->GetKeyPressed(KeyCode::ESC)){
            window->Close();
        }
        if(window->GetKeyPressed(F1)){
            renderer->SetShaderMode();
        }
        if(window->GetKeyPressed(F2)){
            renderer->SetWireframeMode();
        }

        cameraState.x = 0.f;
        cameraState.y = 0.f;
        if(window->GetKeyPressed(KeyCode::W)){
            cameraState.x = 1.f;
        }
        if(window->GetKeyPressed(KeyCode::S)){
            cameraState.x = -1.f;
        }
        if(window->GetKeyPressed(KeyCode::D)){
            cameraState.y = 1.f;
        }
        if(window->GetKeyPressed(KeyCode::A)){
            cameraState.y = -1.f;
        }

        if(window->GetKeyPressed(F1)){
            renderer->SetShaderMode();
        }
        if(window->GetKeyPressed(F2)){
            renderer->SetWireframeMode();
        }

        cameraDirection.x = 0;
        cameraDirection.y = 0;
        if(window->GetMouseButtonPressed(M_RIGHT)){
            window->ShowCursor(false);
            hideCursor = true;
            cameraDirection.x = -static_cast<float>(window->GetMouseXOffset());
            cameraDirection.y = -static_cast<float>(window->GetMouseYOffset());
        }else{
            if(hideCursor){
                hideCursor= false;
                window->ShowCursor(true);
            }
        }
    }

    void GLTFViewer::Update(float elapsedTime) {
        camera->MoveForward(cameraState.x*elapsedTime*10.f);
        camera->MoveRight(cameraState.y*elapsedTime*10.f);
        camera->Rotate(cameraDirection.x,cameraDirection.y);
    }

    void GLTFViewer::Render(Window *window, float elapsedTime) {
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
        glViewport(0,0,window->GetFrameBufferWidth(),window->GetFrameBufferHeight());

        renderer->Render(window,camera,this->models,this->lights, elapsedTime);
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