//
// Created by koerriva on 2021/12/20.
//

#ifndef WX_ENGINE_H
#define WX_ENGINE_H

#include <iostream>
#include <chrono>
#include <thread>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "log.h"
#include "ecs.h"
#include "window.h"
#include "renderer.h"

namespace wx {
    using namespace std::chrono;
    using namespace std;

    //duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch());
    //1ms = 1000000ns
    struct GameTime {
        nanoseconds start;
        nanoseconds now;
        nanoseconds steps;
    };

    class Timer {
    private:
        double lastLoopTime;
    public:
        ~Timer() = default;

        void Init() {
            lastLoopTime = GetTime();
        }

        double GetElapsedTime() {
            double time = GetTime();
            auto elapsedTime = time - lastLoopTime;
            lastLoopTime = time;
            return elapsedTime;
        }

        [[nodiscard]] double GetLastLoopTime() const {
            return lastLoopTime;
        }

        static double GetTime() {
            return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count() / 1000000000.0;
        }
    };

    class App{
    private:
        const float TARGET_FPS = 60;
        const float TARGET_UPS = 60;

    public:
        App();

        App& Setup();

        template<typename... T>
        App& Spawn(T ...components) {
            ::entity_id entity = create_entity(level);

            SpawnBundle(unpack(entity,components)...);

            return *this;
        }

        App& SpawnFromModel(const std::string& model_file){
            std::string filename = "data/"+model_file;
            Assets::LoadAnimateModel(level,filename.c_str());
        }

        App& AddSystem(const char* name,system_t system);

        template<typename T>
        App& InsertResource(T resource) {
            level_insert_share_resource(level,resource);
            return *this;
        }
        void Run();
        App& Cleanup();

    private:
        template<typename T>
        T unpack(::entity_id entity,const T& t) {
            std::cout << "component : " << typeid(T).name() << std::endl;
            level_register_component<T>(level);
            level_add_component(level,entity,t);
            return t;
        }

        template<typename... T>
        void SpawnBundle(T... components){}

        level* level;
        std::vector<std::string> engine_components;
        std::vector<std::string> game_components;

        std::vector<std::string> engine_systems;
        std::vector<std::string> game_systems;
    };
}
#endif //WX_ENGINE_H
