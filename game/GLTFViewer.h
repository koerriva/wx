//
// Created by koerriva on 2021/12/21.
//

#ifndef WX_GLTFVIEWER_H
#define WX_GLTFVIEWER_H

#include "engine.h"

namespace wx {
    struct GLTFViewer {
        Camera* camera;
        Debug* debug;
        vector<::entity_id> models;
        vector<::entity_id> lights;
        ::entity_id canvas;
        bool debug_light_switch = false;
        bool hideCursor = false;
    };
}


#endif //WX_GLTFVIEWER_H
