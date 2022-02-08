//
// Created by koerriva on 2022/1/8.
//

#include "window.h"
#include "systems.h"

namespace wx {
    void input_update_system(level* level,float delta){
        auto* window = level_get_share_resource<Window>(level);
        if(window == nullptr) return;
        auto* inputState = level_get_share_resource<InputState>(level);
        if(inputState == nullptr) return;

        for (int i = 0; i < size(inputState->curr_key_state); ++i) {
            inputState->last_key_state[i]=inputState->curr_key_state[i];
            inputState->curr_key_state[i]=window->GetKeyState(i);
        }

        for (int i = 0; i < size(inputState->curr_button_state); ++i) {
            inputState->last_button_state[i] = inputState->curr_button_state[i];
            inputState->curr_button_state[i] = window->GetMouseState(i);
        }

        double mX,mY;
        window->GetCursorPos(&mX,&mY);
        inputState->last_cursor_pos = inputState->curr_cursor_pos;
        inputState->curr_cursor_pos = vec2(mX,mY);
    }

    void third_person_camera_controller_system(level* level,float delta){
        auto window = level_get_share_resource<Window>(level);
        auto entities_iter = level->entities.begin();
        auto entities_begin = level->entities.begin();
        while (entities_iter != level->entities.end()){
            uint32_t entity_idx = entities_iter-entities_begin;
            entity_id entity = CREATE_ID((*entities_iter),entity_idx);
            if(entity!=0 && level_has_components<Camera,MainCamera>(level,entity)){

                auto * camera = level_get_component<Camera>(level,entity);
                if(camera== nullptr)return;

                auto * inputState = level_get_share_resource<InputState>(level);
                if(inputState== nullptr)return;

                //MoveForward
                if(inputState->GetKeyDown(InputState::KeyCode::W)){
                    camera->position += 2.f * camera->front * delta;
                }
                if(inputState->GetKeyDown(InputState::KeyCode::S)){
                    camera->position += -2.f * camera->front * delta;
                }
                //MoveRight
                if(inputState->GetKeyDown(InputState::KeyCode::D)){
                    camera->position += 2.f * normalize(cross(camera->front, camera->up)) * delta;
                }
                if(inputState->GetKeyDown(InputState::KeyCode::A)){
                    camera->position += -2.f * normalize(cross(camera->front, camera->up)) * delta;
                }
                //Rotation
                vec2 direction{0};
                if(inputState->GetMouseButtonPressed(InputState::MouseCode::M_RIGHT)){
                    window->ShowCursor(false);
                    direction = inputState->GetCursorOffset();
                    camera->yaw += direction.x;
                    camera->pitch -= direction.y;
                    camera->pitch = glm::clamp(camera->pitch, -89.0f, 89.f);
                    glm::vec3 _front;
                    _front.x = cos(glm::radians(camera->pitch)) * cos(glm::radians(camera->yaw));
                    _front.y = sin(glm::radians(camera->pitch));
                    _front.z = cos(glm::radians(camera->pitch)) * sin(glm::radians(camera->yaw));
                    camera->front = glm::normalize(_front);
                }else{
                    window->ShowCursor(true);
                }
            }
            entities_iter++;
        }
    }
}