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
//        auto Plane = Mesh::LoadModelFromGLTF("model\\Plane.glb")[0];
//        Plane.transform.scale += vec3(100);
//        this->models.push_back(Plane);

        auto Scene = Mesh::LoadModelFromGLTF("model\\Scene.gltf");
        for (auto& m:Scene) {
            this->models.push_back(m);
        }

        auto FlySets = Mesh::LoadModelFromGLTF("model\\CesiumDrone.glb");
        for (auto& f:FlySets) {
            f.transform.position += vec3(4);
            this->models.push_back(f);
        }

        auto Sphere = Mesh::LoadModelFromGLTF("model\\Sphere.gltf")[0];
        Sphere.transform.position += vec3(-5,5,0);
        this->models.push_back(Sphere);
        model_t pointLightModel = Sphere;
        pointLightModel.transform.position = vec3(0,6,0);
        this->models.push_back(pointLightModel);
        model_t spotLightModel = Sphere;
        spotLightModel.transform.position = vec3(0,10,0);
        this->models.push_back(spotLightModel);

        camera = new Camera({0.0,2.0,10.0});

        int len;
        const unsigned char * img_buffer = AssetsLoader::LoadTexture("cursor-32-2.png",&len);
        int w,h,comp;
        auto img = stbi_load_from_memory(img_buffer,len,&w,&h,&comp,0);
//        window->SetCursor(img, w, h, 9, 3);
        window->SetCursor(img, w, h, 7, 1);

        uint32_t depth_shader = ShaderProgram::LoadShader("depth");
        uint32_t depth_cube_shader = ShaderProgram::LoadShader("depth_cube",true);

        light_t dirLight;
        dirLight.type = directional;
        dirLight.color = vec3{1.0f};
        dirLight.direction = vec3{0,-1,1};
        dirLight.intensity = 100;
        dirLight.shadow_map = Texture::LoadDepthMap(2048,2048);
        dirLight.shadow_map.shader = depth_shader;
        dirLight.has_shadow_map = 1;
        dirLight.near_plane = -20.f;
        dirLight.far_plane = 20.f;

        dirLight.p = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, dirLight.near_plane, dirLight.far_plane);;
        dirLight.v = glm::lookAt(vec3{-5,5,0}, dirLight.direction*vec3(5.f), glm::vec3(0.0f, 1.0f, 0.0f));
        lights.push_back(dirLight);

        light_t pointLight;
        pointLight.type = point;
        pointLight.color = vec3{1.0f,1.0,1.0};
        pointLight.position = vec3{0.f,6.0,0.0};
        pointLight.intensity = 50;
        pointLight.attenuation = {0.0,0.0,0.12};
        pointLight.shadow_map = Texture::LoadDepthCubeMap(2048,2048);
        pointLight.shadow_map.shader = depth_cube_shader;
        pointLight.has_shadow_map = 1;
        pointLight.near_plane = 1.f;
        pointLight.far_plane = 50.f;
        pointLight.p = glm::perspective(radians(90.f),1.0f,pointLight.near_plane,pointLight.far_plane);
        lights.push_back(pointLight);

        light_t spotLight;
        spotLight.type = spot;
        spotLight.color = vec3{1.0f,1.0,1.0};
        spotLight.position = vec3{0.f,10.,0.};
        spotLight.direction = vec3{0.f,-1.f,0.};
        spotLight.cutoff = glm::cos(radians(45.0f));
        spotLight.intensity = 20;
        spotLight.shadow_map = Texture::LoadDepthMap(2048,2048);
        spotLight.shadow_map.shader = depth_shader;
        spotLight.has_shadow_map = 1;
        spotLight.near_plane = 1.f;
        spotLight.far_plane = 10.f;
        spotLight.p = glm::perspective(radians(90.f),1.0f,spotLight.near_plane,spotLight.far_plane);
        spotLight.v = glm::lookAt(spotLight.position,spotLight.position+spotLight.direction,vec3(0.0f,0.0f,-1.0f));
        lights.push_back(spotLight);

        light_t canvas_light = spotLight;
        canvas.shader = ShaderProgram::LoadShader("hud_debug_depth");
        canvas.vao = Mesh::UnitQuad();
        canvas.texture = canvas_light.shadow_map.texture;
        ShaderProgram::Bind(canvas.shader);
        ShaderProgram::SetInt(canvas.shader,"type",canvas_light.type);
        ShaderProgram::SetFloat(canvas.shader,"near_plane",canvas_light.near_plane);
        ShaderProgram::SetFloat(canvas.shader,"far_plane",canvas_light.far_plane);
        ShaderProgram::Unbind();
        canvas.position = vec2{0};
        canvas.size = vec2{100,100};

        debug = new Debug();
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
            int s = lights[0].state;
            lights[0].state = s==0?1:0;
        }
        if(window->GetKeyPressed(F4)){
            int s = lights[1].state;
            lights[1].state = s==0?1:0;
        }
        if(window->GetKeyPressed(F5)){
            int s = lights[2].state;
            lights[2].state = s==0?1:0;
        }
        if(window->GetKeyPressed(F6)){
            debug_light_switch = !debug_light_switch;
            if(debug_light_switch){
                renderer->SetToLightView(&lights[0]);
            }else{
                renderer->SetToLightView(nullptr);
            }
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

        wchar_t buffer[20];
        swprintf(buffer,L"帧率(ms) %.4f\0",elapsedTime);
        debug->PrintScreen(vec2{window->GetWidth()-120,10},buffer,vec3{0.1,0.999,0.1});
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
    wx::GameEngine engine("GLTFViewer Ver:0.2.3", 1280, 720, true, &game);
    engine.Run();
}