//
// Created by koerriva on 2022/1/8.
//

#ifndef WX_SYSTEMS_H
#define WX_SYSTEMS_H

#include "ecs.h"
#include "components.h"

namespace wx {
    void context_setup_system(level* level,float delta);

    //Window
    void window_update_system(level* level,float delta);
    //Input
    void input_update_system(level* level,float delta);
    //Camera
    void camera_update_system(level* level,float delta);
    //render
    void spatial_update_system(level* level,float delta);
    void render_update_system(level* level,float delta);
    void render_shadow_phase(level* level,float delta,vector<entity_id> models,vector<entity_id> lights);
    void render_mesh_phase(level* level,float delta,vector<entity_id> models,vector<entity_id> lights,::entity_id camera);
    void render_ui_phase(level* level,float delta,const vector<::entity_id>& items);

    //Application
    void third_person_camera_controller_system(level* level,float delta);
}

#endif //WX_SYSTEMS_H
