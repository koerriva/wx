//
// Created by koerriva on 2021/12/21.
//

#include "DummyGame.h"

#define EARTH_RADIUS 1000
namespace wx {

    DummyGame::DummyGame(){
        this->renderer = new Renderer();
        this->timer = new Timer();
    }

    DummyGame::~DummyGame(){
        cout << "Drop DummyGame" << endl;
    }

    void DummyGame::Init(){
        WX_INFO("DummyGame Init...");
        renderer->Init();

        this->baseShader = new ShaderProgram("base");
        meshList.push_back(Mesh::Sphere(EARTH_RADIUS,72,36));

        int len;
        const unsigned char* buffer = AssetsLoader::LoadTexture("earthmap4k.jpg",&len);
        auto tex = Texture(buffer,len);
        textures.push_back(tex);


        this->terrain = new Terrain();
        this->terrain->Init();
        this->terrainShader = new ShaderProgram("base");

        this->camera = new Camera(vec3{0,25,500});
        this->camera->Rotate(0,70);
        cameraLen = length(camera->Position());

        debug = new Debug();

        timer->Init();
    }

    void DummyGame::Input(Window* window){
        if(window->GetKeyPressed(KeyCode::ESC)){
            window->Close();
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
            cameraDirection.x = static_cast<float>(window->GetMouseXOffset());
            cameraDirection.y = static_cast<float>(window->GetMouseYOffset());
        }
    }

    void DummyGame::Update(float interval){
        camera->MoveForward(cameraState.x*interval*10.f);
        camera->MoveRight(cameraState.y*interval*10.f);
        camera->Rotate(cameraDirection.x,cameraDirection.y);
        updateRate = interval;
        cameraLen = length(camera->Position());
        if(cameraLen<25){
            LOD = static_cast<int>(glm::clamp(25.f/cameraLen,1.f,5.f));
        }else{
            LOD = 1;
        }
        terrain->Update(LOD);
    }

    void DummyGame::Render(Window* window,float elapsedTime){
        frameTime += float(timer->GetElapsedTime());
        frameCount += 1;
        if(frameRate==0&&frameTime>0){
            frameRate = int(frameCount/frameTime);
        }

        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
        glViewport(0,0,window->GetFrameBufferWidth(),window->GetFrameBufferHeight());

        renderer->Render(window,camera,meshList,textures,baseShader);
//        renderer->Render(window,camera,terrain,terrainShader);

        std::string text = "帧率:"+to_string(frameRate)+","+to_string(int(1/elapsedTime));
        debug->PrintScreen(vec2{5,5},text.c_str(),vec3{0.05f,.99f,0.05f});

        vec3 camPos = camera->Position();

        text = "相机坐标:"+to_string(camPos.x)+","+to_string(camPos.y)+","+to_string(camPos.z);
        debug->PrintScreen(vec2{5,25},text.c_str(),vec3{0.05f,.99f,0.05f});
        vec3 camRot = camera->Rotation();

        text = "相机角度:"+to_string(camRot.x)+","+to_string(camRot.y)+","+to_string(camRot.z);
        debug->PrintScreen(vec2{5,45},text.c_str(),vec3{0.05f,.99f,0.05f});

        text = "视距:"+to_string(cameraLen);
        debug->PrintScreen(vec2{5,65},text.c_str(),vec3{0.05f,.99f,0.05f});

        text = "LOD:"+to_string(LOD);
        debug->PrintScreen(vec2{5,85},text.c_str(),vec3{0.05f,.99f,0.05f});

        size_t chunks = terrain->GetChunkSize();
        text = "块数:"+to_string(chunks);
        debug->PrintScreen(vec2{5,105},text.c_str(),vec3{0.05f,.99f,0.05f});

        if(frameTime>1.0){
            frameRate = int(frameCount/frameTime);
            frameTime=0;
            frameCount=0;
        }
    }

    void DummyGame::Cleanup(){
        for(auto& mesh:meshList){
            mesh.Cleanup();
        }

        for(auto& tex:textures){
            tex.Cleanup();
        }

        terrain->Cleanup();

        delete terrain;

        delete terrainShader;

        delete camera;

        delete renderer;

        delete timer;
    }
}

int main(int argc,char** argv) {
    system("chcp 65001");

    wx::Log::Init();
    WX_INFO("我的游戏引擎 0.2");
    wx::DummyGame game;
    wx::GameEngine engine("我的游戏引擎 0.2.0", 1280, 720, true, &game);
    engine.Run();
}
