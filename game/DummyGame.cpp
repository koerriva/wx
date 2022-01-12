//
// Created by koerriva on 2021/12/21.
//

#include "DummyGame.h"

namespace wx {
    struct Sun{};

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
                break;
            }
            entities_iter++;
        }

        if(inputState->GetKeyPressed(InputState::K)){
            WX_INFO("Pressed K");
            if(sun){
                sun->state = sun->state==0?1:0;
            }
        }
    }
}

int main(int argc,char** argv) {
    using namespace glm;
    using namespace wx;

    system("chcp 65001");

    wx::Log::Init();
    WX_INFO("我的游戏引擎 0.2");
    auto* app = new wx::App();
    app->InsertResource(wx::WindowConfig{"我的游戏引擎 0.2.0"})
    .Setup()
    .AddSystem(SYSTEM_NAME(wx::test_input_system),wx::test_input_system);

    wx::Canvas canvas{};
    canvas.position = glm::vec2(0,0);
    canvas.size = glm::vec2(100,100);
    canvas.vao = wx::Assets::UnitQuad();

    wx::Camera camera{};
    camera.position = glm::vec3(0.0f,0.0f,5.0f);

    wx::Light sun{};
    sun.type = wx::Light::directional;
    sun.color = vec3{1.0f};
    sun.direction = vec3{0,-1,1};
    sun.intensity = 100;
    sun.shadow_map = wx::TextureLoader::LoadDepthMap(2048, 2048);
    sun.has_shadow_map = 1;
    sun.near_plane = -20.f;
    sun.far_plane = 20.f;

    sun.p = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, sun.near_plane, sun.far_plane);;
    sun.v = glm::lookAt(vec3{-5,5,0}, sun.direction*vec3(5.f), glm::vec3(0.0f, 1.0f, 0.0f));

    app->Spawn(sun,CastShadow{},Sun{});

    app->Spawn(canvas);
    app->Spawn(camera,wx::MainCamera{});
    app->SpawnFromModel("model/Sphere.gltf");

    app->Run();
}
