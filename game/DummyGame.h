//
// Created by koerriva on 2021/12/21.
//

#ifndef WX_DUMMYGAME_H
#define WX_DUMMYGAME_H

#include <engine.h>

namespace wx {
    class DummyGame : public IGameLogic{
    public:
        void Init() override;

        void Input(Window *window) override;

        void Update(float elapsedTime) override;

        void Render(Window *window, float elapsedTime) override;

        void Cleanup() override;
    };
}


#endif //WX_DUMMYGAME_H
