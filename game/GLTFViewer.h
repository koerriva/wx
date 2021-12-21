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

        void Init() override;

        void Input(Window *window) override;

        void Update(float elapsedTime) override;

        void Render(Window *window, float elapsedTime) override;

        void Cleanup() override;

    private:
        Renderer* renderer;
        Camera* camera;
        vector<model_t> models;
    };
}


#endif //WX_GLTFVIEWER_H
