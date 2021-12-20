//
// Created by koerriva on 2021/12/20.
//

#include "engine.h"
#include "assets.h"
#include "font.h"

namespace wx {
    GameEngine::GameEngine(const char *title, int width, int height, bool vsync, IGameLogic *game) {
        this->game = game;
        this->window = new Window(title,width,height,vsync);
        this->timer = new Timer();
    }

    GameEngine::~GameEngine()
    {
        cout << "Drop GameEngine" << endl;
    }

    void GameEngine::Init(){
        timer->Init();
        //窗口先初始化
        window->Init();
        //加载资源
        ResourceLoader::Init();
        //加载字体
        Font::Init();
        //加载游戏
        game->Init();
    }

    void GameEngine::Run(){
        Init();
        float elapsedTime;
        float acc = 0.f;
        float interval = 1.f/TARGET_UPS;
        while (!window->Closed())
        {
            elapsedTime = float(timer->GetElapsedTime());
            acc += elapsedTime;
            Input();
            if(!window->VSynced()){
                while (acc >= interval){
                    Update(interval);
                    acc -= interval;
                }
            }else{
                Update(elapsedTime);
            }

            Render(elapsedTime);
            if(!window->VSynced()){
                Sync();
            }
        }
        Cleanup();
    }

    void GameEngine::Sync() {
        auto loopSlot = 1.f/TARGET_FPS;
        auto endTime = timer->GetLastLoopTime()+loopSlot;
        auto now = Timer::GetTime();
        while(now<endTime){
            this_thread::sleep_for(milliseconds(1));
            now = Timer::GetTime();
        }
    }

    void GameEngine::Input(){
        game->Input(window);
    }

    void GameEngine::Update(float elapsedTime){
        window->Update();
        game->Update(elapsedTime);
    }

    void GameEngine::Render(float elapsedTime){
        game->Render(window,elapsedTime);
    }

    void GameEngine::Cleanup(){
        game->Cleanup();

        ResourceLoader::Cleanup();
        Font::Cleanup();

        //窗口必须最后清理，防止OpenGL Context关闭。
        window->Cleanup();
    }
}