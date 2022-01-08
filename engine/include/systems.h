//
// Created by koerriva on 2022/1/8.
//

#ifndef WX_SYSTEMS_H
#define WX_SYSTEMS_H

#include "ecs.h"
#include "components.h"

namespace wx {
    //Window
    void window_update_system(level* level,float delta);
    //Input
    void input_update_system(level* level,float delta);
    //Camera
    void camera_update_system(level* level,float delta);
    //render
    void render_update_system(level* level,float delta);
    void render_shadow_phase_system(level* level,float delta,vector<entity_id> models,vector<entity_id> lights);
    void render_mesh_phase_system(level* level,float delta,vector<entity_id> models,vector<entity_id> lights);
    void render_ui_phase_system(level* level,float delta,vector<::entity_id> items);

    //Application
    void third_person_camera_controller_system(level* level,float delta);
}

#endif //WX_SYSTEMS_H
