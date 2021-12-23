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

    class IGameLogic{
    public:
        virtual void Init(Window *window) = 0;
        virtual void Input(Window* window) = 0;
        virtual void Update(float elapsedTime) = 0;
        virtual void Render(Window* window,float elapsedTime) = 0;
        virtual void Cleanup() = 0;
    };

    class GameEngine {
    private:
        const float TARGET_FPS = 60;
        const float TARGET_UPS = 30;

        Window *window;
        IGameLogic *game;
        Timer *timer;
    public:
        GameEngine(const char *title, int width, int height, bool vsync, IGameLogic *game);
        ~GameEngine();
        void Run();
    protected:
        void Init();
        void Input();
        void Update(float elapsedTime);
        void Render(float elapsedTime);
        void Sync();
        void Cleanup();
    };
}
#endif //WX_ENGINE_H
