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
//        auto Plane = Renderer::LoadModelFromGLTF("model\\Plane.glb")[0];
//        Plane.transform.scale += vec3(100);
//        this->models.push_back(Plane);

        auto Scene = Renderer::LoadModelFromGLTF("model\\Scene.gltf");
        for (auto& m:Scene) {
            this->models.push_back(m);
        }

        auto FlySets = Renderer::LoadModelFromGLTF("model\\CesiumDrone.glb");
        for (auto& f:FlySets) {
            f.transform.position += vec3(5);
            this->models.push_back(f);
        }

        auto Sphere = Renderer::LoadModelFromGLTF("model\\Sphere.gltf")[0];
        Sphere.transform.position += vec3(-5,5,0);
        this->models.push_back(Sphere);
        model_t pointLightModel = Sphere;
        pointLightModel.transform.position = vec3(0,10,0);
        this->models.push_back(pointLightModel);

        camera = new Camera({0.0,2.0,10.0});

        int len;
        const unsigned char * img_buffer = AssetsLoader::LoadTexture("cursor-32-2.png",&len);
        int w,h,comp;
        auto img = stbi_load_from_memory(img_buffer,len,&w,&h,&comp,0);
//        window->SetCursor(img, w, h, 9, 3);
        window->SetCursor(img, w, h, 7, 1);

        uint32_t depth_shader = ShaderProgram::LoadShader("depth");
        light_t dirLight;
        dirLight.type = directional;
        dirLight.color = vec3{1.0f};
        dirLight.direction = vec3{0,-1,1};
        dirLight.intensity = 5;
        dirLight.shadow_map = Texture::LoadDepthMap(4096,4096);
        dirLight.shadow_map.shader = depth_shader;
        dirLight.has_shadow_map = 1;

        glm::mat4 P(1.0f),V(1.0f);
        float near_plane = -1.f, far_plane = 20.f;
        P = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        vec3 lightPos = vec3{-5,5,0};
        V = glm::lookAt(lightPos, dirLight.direction*vec3(5.f), glm::vec3(0.0f, 1.0f, 0.0f));
        dirLight.p = P;
        dirLight.v = V;
//        lights.push_back(dirLight);

        light_t pointLight;
        pointLight.type = point;
        pointLight.color = vec3{1.0f,0.2,0.2};
        pointLight.position = vec3{0,10,0};
        pointLight.intensity = 100;
        pointLight.attenuation = {0.0,0.0,0.12};
        pointLight.shadow_map = Texture::LoadDepthMap(4096,4096);
        pointLight.shadow_map.shader = depth_shader;
        pointLight.has_shadow_map = 1;
        pointLight.p = glm::perspective(radians(60.0f),1.0f,0.1f,100.f);
        lights.push_back(pointLight);

        canvas.shader = ShaderProgram::LoadShader("hud");
        canvas.vao = Mesh::UnitQuad();
        canvas.texture = pointLight.shadow_map.texture;
        canvas.position = vec2{0};
        canvas.size = vec2{200,200};

//        light_t spotLight;
//        spotLight.type = spot;
//        spotLight.color = vec3{1.0f,1.0,1.0};
//        spotLight.position = vec3{0.f,10.,0.};
//        spotLight.direction = vec3{0.f,-1.f,0.};
//        spotLight.cutoff = glm::cos(radians(60.0f));
//        spotLight.intensity = 10;
//        lights.push_back(spotLight);
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
        if(window->GetKeyPressed(F3)){
            int has = lights[0].has_shadow_map;
            lights[0].has_shadow_map = has==0?1:0;
        }

        cameraState.x = 0.f;
        cameraState.y = 0.f;
        if(window->GetKeyDown(KeyCode::W)){
            cameraState.x = 1.f;
        }
        if(window->GetKeyDown(KeyCode::S)){
            cameraState.x = -1.f;
        }
        if(window->GetKeyDown(KeyCode::D)){
            cameraState.y = 1.f;
        }
        if(window->GetKeyDown(KeyCode::A)){
            cameraState.y = -1.f;
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
        renderer->Render(window,models,lights,elapsedTime);
        renderer->Render(window,camera,this->models,this->lights,this->canvas, elapsedTime);
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