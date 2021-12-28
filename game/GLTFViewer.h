//
// Created by koerriva on 2021/12/21.
//

#ifndef WX_GLTFVIEWER_H
#define WX_GLTFVIEWER_H

#include "engine.h"

namespace wx {
    class GLTFViewer : public IGameLogic {
    public:
        GLTFViewer();
        ~GLTFViewer();

        void Init(Window *window) override;

        void Input(Window *window) override;

        void Update(float elapsedTime) override;

        void Render(Window *window, float elapsedTime) override;

        void Cleanup() override;

    private:
        Renderer* renderer;
        Camera* camera;
        Debug* debug;
        vector<model_t> models;
        vector<light_t> lights;
        canvas_t canvas;

        bool debug_light_switch = false;

        vec2 cameraState {0.f,0.f};
        vec2 cameraDirection{0.f,0.f};
        float cameraLen = 0;

        bool hideCursor = false;
    };
}


#endif //WX_GLTFVIEWER_H
