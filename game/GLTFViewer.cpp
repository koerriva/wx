//
// Created by koerriva on 2021/12/21.
//

#include <stb/stb_image.h>
#include "GLTFViewer.h"

using namespace glm;

#include "systems.h"
#include "menu_loader.h"

#define MAX_VERTEX_BUFFER (512 * 1024)
#define MAX_ELEMENT_BUFFER (128 * 1024)

namespace wx {

    void gltf_viewer_game(App* app){
        app->InsertResource(WindowConfig{"Metaverse Engine 0.3.2",1440,900});
        WX_INFO("_1-------------------------------------");
        NKMenu menu = load_menu("hud\\GLTFViewer.xml");
        app->InsertResource(menu);

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
        sun.intensity = 10;
        sun.shadow_map = wx::TextureLoader::LoadDepthMap(4096, 4096);
        sun.has_shadow_map = 1;
        sun.near_plane = -1.f;
        sun.far_plane = 20.f;

        sun.p = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, sun.near_plane, sun.far_plane);
        sun.v = glm::lookAt(sun.position, vec3(0.0f), glm::vec3(1.0f));

        app->Spawn(sun,CastShadow{},Sun{});
        app->Spawn(canvas);
        app->Spawn(camera,wx::MainCamera{});

        quat dir = quatLookAt(sun.direction,vec3(0.0f,1.0f,0.0f));
        auto fly01_entity = app->SpawnFromModel("model\\CesiumDrone.glb","Fly01",Transform{.position=vec3(0.f,5.f,0.0f)});
        app->AddComponent(fly01_entity,LuaScript{"script\\enemy.controller.lua"});
//        app->SpawnFromModel("model\\cube.gltf","Cube01");
//        app->SpawnFromModel("model\\Plane.glb","Plane01",Transform{.scale=vec3(20.f)});
        app->SpawnFromModel("model\\Axis.glb","SunGizmos",Transform{.position=sun.position,.rotation=dir});
        app->SpawnFromModel("model\\Snake.gltf","Snake01",Transform{.position=vec3(0.f,0.f,3.f),.scale=vec3(0.2f)});
        app->SpawnFromModel("model\\Scene.gltf","Scene");
        auto player01_entity = app->SpawnFromModel("model\\Formal.gltf","Player01");
        app->AddComponent(player01_entity,LuaScript{"script\\player.controller.lua"});

        app->AddSystem(SYSTEM_NAME(test_input_system),test_input_system);
        app->AddSystem(SYSTEM_NAME(third_person_camera_controller_system),third_person_camera_controller_system);
        WX_INFO("_run-------------------------------------");
    }

    void test_input_system(level* level,float delta){
        auto inputState = level_get_share_resource<InputState>(level);

        Light* sun = nullptr;
        Animator* snake_animator = nullptr;
        Animator* fly_animator = nullptr;
        Animator* player_animator = nullptr;

        auto entities_iter = level->entities.begin();
        auto entities_begin = level->entities.begin();

        while (entities_iter != level->entities.end()){
            uint32_t entity_idx = entities_iter-entities_begin;
            entity_id entity = CREATE_ID((*entities_iter),entity_idx);
            if(entity!=0 && level_has_components<Sun>(level,entity)){
                sun = level_get_component<Light>(level,entity);
            }
            if(entity!=0 && level_has_components<Spatial3d,Animator>(level,entity)){
                auto spatial = level_get_component<Spatial3d>(level,entity);
                auto animator = level_get_component<Animator>(level,entity);
                if(spatial->name=="Snake01"){
                    snake_animator = animator;
                }
                if(spatial->name=="Fly01"){
                    fly_animator = animator;
                }
                if(spatial->name=="Player01"){
                    player_animator = animator;
                }
            }
            entities_iter++;
        }

        if(inputState->GetKeyPressed(InputState::K)){
            WX_INFO("Pressed K");
            if(sun){
                sun->state = sun->state==0?1:0;
            }
        }

        if(inputState->GetKeyPressed(InputState::P)){
            WX_INFO("Pressed P");
            if(snake_animator){
                if(snake_animator->state!=Animator::play){
                    WX_INFO("Play Snake Animation");
                    snake_animator->Play("Snake_Idle");
                }else{
                    WX_INFO("Pause Snake Animation");
                    snake_animator->Pause();
                }
            }
            if(fly_animator){
                if(fly_animator->state!=Animator::play){
                    WX_INFO("Play Fly Animation");
                    fly_animator->Play();
                }else{
                    WX_INFO("Pause Fly Animation");
                    fly_animator->Pause();
                }
            }
            if(player_animator){
                if(player_animator->state!=Animator::play){
                    WX_INFO("Play Player Animation");
                    player_animator->Play("Walk");
                }else{
                    WX_INFO("Pause Player Animation");
                    player_animator->Pause();
                }
            }
        }

        if(inputState->GetKeyPressed(InputState::L)){
            WX_INFO("Pressed S");
            if(snake_animator){
                WX_INFO("Stop Snake Animation");
                snake_animator->Stop();
            }
            if(fly_animator){
                WX_INFO("Stop Fly Animation");
                fly_animator->Stop();
            }
            if(player_animator){
                WX_INFO("Stop Player Animation");
            }
        }

        if(inputState->GetKeyPressed(InputState::Num1)){
            WX_INFO("Pressed Num1");
            if(level_has_share_resource<RenderState>(level)){
                auto render_state = level_get_share_resource<RenderState>(level);
                if(render_state->mode==RenderState::Shader){
                    render_state->mode=RenderState::Wireframe;
                }else{
                    render_state->mode=RenderState::Shader;
                }
            }
        }
    }

    void nuklear_update_system(level* level,float delta){
        if(!level_has_share_resource<NuklearContext>(level))return;
        auto nkcontext = level_get_share_resource<NuklearContext>(level);
        auto glfw = (struct nk_glfw*)nkcontext->glfw;
        auto ctx = (struct nk_context*)nkcontext->ctx;
        if(!level_has_share_resource<NKMenu>(level))return;
        auto nkMenu = level_get_share_resource<NKMenu>(level);

        Light* sun = nullptr;
        Animator* snake_animator = nullptr;
        Animator* fly_animator = nullptr;
        Animator* player_animator = nullptr;
        auto entities_iter = level->entities.begin();
        auto entities_begin = level->entities.begin();

        while (entities_iter != level->entities.end()){
            uint32_t entity_idx = entities_iter-entities_begin;
            entity_id entity = CREATE_ID((*entities_iter),entity_idx);
            if(entity!=0 && level_has_components<Sun>(level,entity)){
                sun = level_get_component<Light>(level,entity);
            }
            if(entity!=0 && level_has_components<Spatial3d,Animator>(level,entity)){
                auto spatial = level_get_component<Spatial3d>(level,entity);
                auto animator = level_get_component<Animator>(level,entity);
                if(spatial->name=="Snake01"){
                    snake_animator = animator;
                }
                if(spatial->name=="Fly01"){
                    fly_animator = animator;
                }
                if(spatial->name=="Player01"){
                    player_animator = animator;
                }
            }
            entities_iter++;
        }

        nk_glfw3_new_frame(glfw);
        render_menu(ctx,*nkMenu);
        sun->state = std::get<int>(nkMenu->data["sun.turn"]);
        int player_animate = std::get<int>(nkMenu->data["player.animate"]);
        if(player_animate==1){
            if(player_animator->state!=Animator::play){
                player_animator->Play("Walk");
            }
        }else{
            if(player_animator->state!=Animator::stop){
                player_animator->Stop();
            }
        }
        nk_glfw3_render(glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
    }
}

int main(int argc,char** argv) {

//    system("chcp 65001");

    wx::Log::Init();
    wx::AssetsLoader::Init();
    wx::ScriptEngine::Init();

    WX_INFO("资源加载完成");
    auto* app = new wx::App();
    app->AddPlugin(SYSTEM_NAME(wx::gltf_viewer_game),wx::gltf_viewer_game);
    app->Run();
}