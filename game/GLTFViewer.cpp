//
// Created by koerriva on 2021/12/21.
//

#include <stb/stb_image.h>
#include "GLTFViewer.h"

using namespace glm;

#include "systems.h"

namespace wx {

    void gltf_viewer_game(App* app){
        app->InsertResource(WindowConfig{"Metaverse Engine 0.2.4",1920,1080});
        WX_INFO("_1-------------------------------------");
        wx::Canvas canvas{};
        canvas.position = glm::vec2(0,0);
        canvas.size = glm::vec2(100,100);
        canvas.vao = wx::Assets::UnitQuad();

        wx::Camera camera{};
        camera.position = glm::vec3(0.0f,2.0f,10.0f);

        wx::Light sun{};
        sun.type = wx::Light::directional;
        sun.color = vec3{1.0f};
        sun.position = vec3(5.f,5.f,0.f);
        sun.direction = normalize(-sun.position);
        sun.intensity = 50;
        sun.shadow_map = wx::TextureLoader::LoadDepthMap(4096, 4096);
        sun.has_shadow_map = 1;
        sun.near_plane = -20.f;
        sun.far_plane = 40.f;

        sun.p = glm::ortho(-40.0f, 40.0f, -40.0f, 40.0f, sun.near_plane, sun.far_plane);;
        sun.v = glm::lookAt(sun.position, vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        app->Spawn(sun,CastShadow{},Sun{});
        app->Spawn(canvas);
        app->Spawn(camera,wx::MainCamera{});
        quat dir = quatLookAt(sun.direction,vec3(0.0f,1.0f,0.0f));
        app->SpawnFromModel("model\\CesiumDrone.glb","Fly",Transform{.position=vec3(1.f,3.f,0.0f)});
//        app->SpawnFromModel("model\\Plane.glb","Plane",Transform{.scale=vec3(20.f)});
//        app->SpawnFromModel("model\\Axis.glb","SunGizmos",Transform{.position=sun.position,.rotation=dir});
//        app->SpawnFromModel("model\\Snake.gltf","Snake");
        app->SpawnFromModel("model\\Scene.gltf","Scene");


        app->AddSystem(SYSTEM_NAME(test_input_system),test_input_system);
        app->AddSystem(SYSTEM_NAME(third_person_camera_controller_system),third_person_camera_controller_system);
        WX_INFO("_run-------------------------------------");
    }

    void test_input_system(level* level,float delta){
        auto inputState = level_get_share_resource<InputState>(level);

        Light* sun = nullptr;
        auto entities_iter = level->entities.begin();
        auto entities_begin = level->entities.begin();

        while (entities_iter != level->entities.end()){
            uint32_t entity_idx = entities_iter-entities_begin;
            entity_id entity = CREATE_ID((*entities_iter),entity_idx);
            if(entity!=0 && level_has_components<Sun>(level,entity)){
                sun = level_get_component<Light>(level,entity);
            }
//            if(entity!=0 && level_has_components<Mesh,Transform>(level,entity)){
//                auto transform = level_get_component<Transform>(level,entity);
//                transform->rotation *= quat(vec3(radians(1.f), radians(1.0f),0.f));
//            }
            entities_iter++;
        }

        if(inputState->GetKeyPressed(InputState::K)){
            WX_INFO("Pressed K");
            if(sun){
                sun->state = sun->state==0?1:0;
            }
        }
    }

//    void GLTFViewer::Init(Window *window) {
//        WX_INFO("GLFTViewer Init");
//        renderer->Init();
////        auto Plane = MeshLoader::LoadModelFromGLTF("model\\Plane.glb")[0];
////        Plane.transform.scale += vec3(100);
////        this->models.push_back(Plane);
//
//        auto Snake = MeshLoader::LoadModelFromGLTF("model\\Snake.gltf");
//        for (auto& m:Snake) {
//            this->models.push_back(m);
//        }
//
////        auto Scene = MeshLoader::LoadModelFromGLTF("model\\Scene.gltf");
////        for (auto& m:Scene) {
////            this->models.push_back(m);
////        }
//
////        auto FlySets = MeshLoader::LoadModelFromGLTF("model\\CesiumDrone.glb");
////        for (auto& f:FlySets) {
////            f.transform.position += vec3(4);
////            this->models.push_back(f);
////        }
//
////        auto Sphere = MeshLoader::LoadModelFromGLTF("model\\Sphere.gltf")[0];
////        Sphere.transform.position += vec3(-5,5,0);
////        this->models.push_back(Sphere);
////        model_t pointLightModel = Sphere;
////        pointLightModel.transform.position = vec3(0,6,0);
////        this->models.push_back(pointLightModel);
////        model_t spotLightModel = Sphere;
////        spotLightModel.transform.position = vec3(0,10,0);
////        this->models.push_back(spotLightModel);
//
////        auto arm = MeshLoader::LoadModelFromGLTF("model\\arm_skin.gltf")[0];
////        arm.transform.scale += vec3(10);
////        this->models.push_back(arm);
//
//        camera = new Camera({0.0,2.0,10.0});
//
//        int len;
//        const unsigned char * img_buffer = AssetsLoader::LoadTexture("cursor-32-2.png",&len);
//        int w,h,comp;
//        auto img = stbi_load_from_memory(img_buffer,len,&w,&h,&comp,0);
////        window->SetCursor(img, w, h, 9, 3);
//        window->SetCursor(img, w, h, 7, 1);
//
//        uint32_t depth_shader = ShaderProgram::LoadShader("depth");
//        uint32_t depth_cube_shader = ShaderProgram::LoadShader("depth_cube",true);
//
//        light_t dirLight;
//        dirLight.type = directional;
//        dirLight.color = vec3{1.0f};
//        dirLight.direction = vec3{0,-1,1};
//        dirLight.intensity = 100;
//        dirLight.shadow_map = TextureLoader::LoadDepthMap(2048, 2048);
//        dirLight.shadow_map.shader = depth_shader;
//        dirLight.has_shadow_map = 1;
//        dirLight.near_plane = -20.f;
//        dirLight.far_plane = 20.f;
//
//        dirLight.p = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, dirLight.near_plane, dirLight.far_plane);;
//        dirLight.v = glm::lookAt(vec3{-5,5,0}, dirLight.direction*vec3(5.f), glm::vec3(0.0f, 1.0f, 0.0f));
//        lights.push_back(dirLight);
//
//        light_t pointLight;
//        pointLight.type = point;
//        pointLight.color = vec3{1.0f,1.0,1.0};
//        pointLight.position = vec3{0.f,6.0,0.0};
//        pointLight.intensity = 50;
//        pointLight.attenuation = {0.0,0.0,0.12};
//        pointLight.shadow_map = TextureLoader::LoadDepthCubeMap(2048, 2048);
//        pointLight.shadow_map.shader = depth_cube_shader;
//        pointLight.has_shadow_map = 1;
//        pointLight.near_plane = 1.f;
//        pointLight.far_plane = 50.f;
//        pointLight.p = glm::perspective(radians(90.f),1.0f,pointLight.near_plane,pointLight.far_plane);
//        lights.push_back(pointLight);
//
//        light_t spotLight;
//        spotLight.type = spot;
//        spotLight.color = vec3{1.0f,1.0,1.0};
//        spotLight.position = vec3{0.f,10.,0.};
//        spotLight.direction = vec3{0.f,-1.f,0.};
//        spotLight.cutoff = glm::cos(radians(45.0f));
//        spotLight.intensity = 20;
//        spotLight.shadow_map = TextureLoader::LoadDepthMap(2048, 2048);
//        spotLight.shadow_map.shader = depth_shader;
//        spotLight.has_shadow_map = 1;
//        spotLight.near_plane = 1.f;
//        spotLight.far_plane = 10.f;
//        spotLight.p = glm::perspective(radians(90.f),1.0f,spotLight.near_plane,spotLight.far_plane);
//        spotLight.v = glm::lookAt(spotLight.position,spotLight.position+spotLight.direction,vec3(0.0f,0.0f,-1.0f));
//        lights.push_back(spotLight);
//
//        light_t canvas_light = spotLight;
//        canvas.shader = ShaderProgram::LoadShader("hud_debug_depth");
//        canvas.vao = MeshLoader::UnitQuad();
//        canvas.texture = canvas_light.shadow_map.texture;
//        ShaderProgram::Bind(canvas.shader);
//        ShaderProgram::SetInt(canvas.shader,"type",canvas_light.type);
//        ShaderProgram::SetFloat(canvas.shader,"near_plane",canvas_light.near_plane);
//        ShaderProgram::SetFloat(canvas.shader,"far_plane",canvas_light.far_plane);
//        ShaderProgram::Unbind();
//        canvas.position = vec2{0};
//        canvas.size = vec2{100,100};
//
//        debug = new Debug();
//    }
//
//    void GLTFViewer::Input(Window *window) {
//        if(window->GetKeyPressed(KeyCode::ESC)){
//            window->Close();
//        }
//        if(window->GetKeyPressed(F1)){
//            renderer->SetShaderMode();
//        }
//        if(window->GetKeyPressed(F2)){
//            renderer->SetWireframeMode();
//        }
//        if(window->GetKeyPressed(F3)){
//            int s = lights[0].state;
//            lights[0].state = s==0?1:0;
//        }
//        if(window->GetKeyPressed(F4)){
//            int s = lights[1].state;
//            lights[1].state = s==0?1:0;
//        }
//        if(window->GetKeyPressed(F5)){
//            int s = lights[2].state;
//            lights[2].state = s==0?1:0;
//        }
//        if(window->GetKeyPressed(F6)){
//            debug_light_switch = !debug_light_switch;
//            if(debug_light_switch){
//                renderer->SetToLightView(&lights[0]);
//            }else{
//                renderer->SetToLightView(nullptr);
//            }
//        }
//    }
//
//    void GLTFViewer::Update(float elapsedTime) {
//        camera->MoveForward(cameraState.x*elapsedTime*10.f);
//        camera->MoveRight(cameraState.y*elapsedTime*10.f);
//        camera->Rotate(cameraDirection.x,cameraDirection.y);
//    }
//
//    void GLTFViewer::Render(Window *window, float elapsedTime) {
//        renderer->Render(window,models,lights,elapsedTime);
//        renderer->Render(window,camera,this->models,this->lights,this->canvas, elapsedTime);
//
//        wchar_t buffer[20];
//        swprintf(buffer,L"帧率(ms) %.4f\0",elapsedTime);
//        debug->PrintScreen(vec2{window->GetWidth()-120,10},buffer,vec3{0.1,0.999,0.1});
//    }
//
//    void GLTFViewer::Cleanup() {
//        delete camera;
//        delete renderer;
//    }
}

int main(int argc,char** argv) {

    system("chcp 65001");

    wx::Log::Init();
    wx::AssetsLoader::Init();

    WX_INFO("我的游戏引擎 0.2");
    auto* app = new wx::App();
    app->AddPlugin(SYSTEM_NAME(wx::gltf_viewer_game),wx::gltf_viewer_game);
    app->Run();
}