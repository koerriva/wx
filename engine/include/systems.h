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
    void nuklear_update_system(level* level,float delta);
    //Input
    void input_update_system(level* level,float delta);
    //Camera
    void camera_update_system(level* level,float delta);
    //render
    void spatial_update_system(level* level,float delta);
    void render_update_system(level* level,float delta);
    void render_skybox_phase(level* level,float delta);
    void render_skydome_phase(level* level,float delta);
    void render_shadow_phase(level* level,float delta);
    void render_mesh_phase(level* level,float delta);
    void render_ui_phase(level* level,float delta);
    void render_nuklear_phase(level* level,float delta);
    //animation
    void animator_setup_system(level* level,float delta);
    void animator_update_system(level* level,float delta);
    //script
    void script_setup_system(level* level,float delta);
    void script_update_system(level* level,float delta);

    //Application
    void third_person_camera_controller_system(level* level,float delta);
}

#endif //WX_SYSTEMS_H
