//
// Created by koerriva on 2021/12/21.
//

#ifndef WX_DUMMYGAME_H
#define WX_DUMMYGAME_H

#include <engine.h>

namespace wx {
    class DummyGame : public IGameLogic{
    public:
        DummyGame();
        ~DummyGame();

        void Init(Window *window) override;

        void Input(Window *window) override;

        void Update(float elapsedTime) override;

        void Render(Window *window, float elapsedTime) override;

        void Cleanup() override;

    private:
        Renderer* renderer;
        Camera* camera = nullptr;
        Debug* debug = nullptr;
        vector<Mesh> meshList;
        vector<Texture> textures;
        Timer* timer;
        Terrain* terrain;
        ShaderProgram baseShader;
        ShaderProgram terrainShader;

        float updateRate = 0.f;
        float frameTime = 0.f;
        int frameCount = 0;
        int frameRate = 0;
        vec2 cameraState {0.f,0.f};
        vec2 cameraDirection{0.f,0.f};
        float cameraLen = 0;
        int LOD = 1;
    };
}


#endif //WX_DUMMYGAME_H
