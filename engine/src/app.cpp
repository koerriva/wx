//
// Created by koerriva on 2022/1/10.
//

#include "engine.h"
#include "systems.h"
#include "font.h"

namespace wx {
    App::App() {
        level = level_create();

        level_register_component<Camera>(level);
        level_register_component<MainCamera>(level);
        level_register_component<Transform>(level);
        level_register_component<AnimatedTransform>(level);
        level_register_component<CastShadow>(level);
        level_register_component<ReceiveShadow>(level);
        level_register_component<Light>(level);
        level_register_component<Joint>(level);
        level_register_component<Mesh>(level);
        level_register_component<Canvas>(level);
        level_register_component<Spatial3d>(level);

        engine_components.emplace_back(typeid(Camera).name());
        engine_components.emplace_back(typeid(MainCamera).name());
        engine_components.emplace_back(typeid(Transform).name());
        engine_components.emplace_back(typeid(AnimatedTransform).name());
        engine_components.emplace_back(typeid(CastShadow).name());
        engine_components.emplace_back(typeid(ReceiveShadow).name());
        engine_components.emplace_back(typeid(Light).name());
        engine_components.emplace_back(typeid(Joint).name());
        engine_components.emplace_back(typeid(Mesh).name());
        engine_components.emplace_back(typeid(Canvas).name());
        engine_components.emplace_back(typeid(Spatial3d).name());

        level_register_system(level,window_update_system, SYSTEM_NAME(window_update_system));
        level_register_system(level,input_update_system, SYSTEM_NAME(input_update_system));
        level_register_system(level,camera_update_system, SYSTEM_NAME(camera_update_system));
        level_register_system(level,render_update_system, SYSTEM_NAME(render_update_system));

        engine_systems.emplace_back(SYSTEM_NAME(window_update_system));
        engine_systems.emplace_back(SYSTEM_NAME(input_update_system));
        engine_systems.emplace_back(SYSTEM_NAME(camera_update_system));
        engine_systems.emplace_back(SYSTEM_NAME(render_update_system));
    }

    App& App::Setup(){
        auto windowConfig = level_get_share_resource<WindowConfig>(level);
        if(windowConfig == nullptr){
            level_insert_share_resource(level,WindowConfig{"",1280,720,true});
            windowConfig = level_get_share_resource<WindowConfig>(level);
        }
        level_insert_share_resource(level, Window{windowConfig->title,
                                                  windowConfig->width,windowConfig->height,
                                                  windowConfig->vSync});
        auto window = level_get_share_resource<Window>(level);
        window->Init();

        AssetsLoader::Init();
        Font::Init();

        level_insert_share_resource(level,VPMatrices{});
        level_insert_share_resource(level,FrameState{});
        level_insert_share_resource(level,InputState{});

        uint32_t pbrShader = ShaderProgram::LoadShader("pbr");
        level_insert_share_resource(level,PBRShader{pbrShader});
        uint32_t flatShader = ShaderProgram::LoadShader("hud");
        level_insert_share_resource(level,FlatShader{flatShader});
        uint32_t fontShader = ShaderProgram::LoadShader("font");
        level_insert_share_resource(level,FontShader{fontShader});
        uint32_t depthShader = ShaderProgram::LoadShader("depth");
        level_insert_share_resource(level,DepthShader{depthShader});
        uint32_t depthCubeShader = ShaderProgram::LoadShader("depth_cube", true);
        level_insert_share_resource(level,DepthCubeShader{depthCubeShader});

        return *this;
    }

    App& App::AddSystem(const char* name,system_t system) {
        level_register_system(level,system, name);
        game_systems.emplace_back(name);
        return *this;
    }

    void App::Run() {
        auto window = level_get_share_resource<Window>(level);

        auto sync_timer = new Timer;
        sync_timer->Init();
        auto frame_timer = new Timer;
        frame_timer->Init();

        auto Sync = [this,sync_timer](){
            auto loopSlot = 1.f/TARGET_FPS;
            auto endTime = sync_timer->GetLastLoopTime()+loopSlot;
            auto now = Timer::GetTime();
            while(now<endTime){
                this_thread::sleep_for(milliseconds(1));
                now = Timer::GetTime();
            }
        };

        while (!window->Closed()){
            float frameTime = frame_timer->GetElapsedTime();

            for (const auto& engine_system:engine_systems) {
                level->systems[engine_system](level,frameTime);
            }

            for (const auto& game_system:game_systems) {
                level->systems[game_system](level,frameTime);
            }

            if(!window->VSynced()){
                Sync();
            }
        }

        AssetsLoader::Cleanup();
        Font::Cleanup();
    }

    App& App::Cleanup() {
        level_destroy(level);
        return *this;
    }
}