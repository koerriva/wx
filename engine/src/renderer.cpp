//
// Created by koerriva on 2021/12/21.
//

#include <iostream>
#include "log.h"
#include "renderer.h"
#include "font.h"
#include "systems.h"

namespace wx {
    void camera_update_system(level* level,float delta){
        auto* window = level_get_share_resource<Window>(level);
        auto* matrix = level_get_share_resource<VPMatrices>(level);
        if(matrix== nullptr){
            WX_CORE_ERROR("[Update VPMatrices Err] : VPMatrices not found");
        }

        auto entities_iter = level->entities.begin();
        auto entities_begin = level->entities.begin();
        while (entities_iter != level->entities.end()){
            uint32_t entity_idx = entities_iter-entities_begin;
            entity_id entity = CREATE_ID((*entities_iter),entity_idx);
            if(entity!=0 && level_has_components<Camera,MainCamera>(level,entity)){
                auto * camera = level_get_component<Camera>(level,entity);
                //计算观察矩阵
                matrix->view = lookAt(camera->position,camera->position+camera->front,camera->up);
            }
            entities_iter++;
        }

        matrix->project = perspective(radians(60.f),window->GetAspect(),0.1f,1000.f);
        matrix->ortho = ortho(0.f,float(window->GetWidth()),float(window->GetHeight()),0.f);
    }

    mat4 calcGlobalMatrix(level* level,::entity_id entity){
        auto spatial = level_get_component<Spatial3d>(level,entity);

        if(spatial->parent>0){
            if(level_has_components<AnimatedTransform>(level,entity)){
                auto transform = level_get_component<AnimatedTransform>(level,entity);
                return calcGlobalMatrix(level,spatial->parent)*transform->GetLocalMatrix();
            }
            if(level_has_components<Transform>(level,entity)){
                auto transform = level_get_component<Transform>(level,entity);
                return calcGlobalMatrix(level,spatial->parent)*transform->GetLocalMatrix();
            }
        }else{
            if(level_has_components<AnimatedTransform>(level,entity)){
                auto transform = level_get_component<AnimatedTransform>(level,entity);
                return transform->GetLocalMatrix();
            }
            if(level_has_components<Transform>(level,entity)){
                auto transform = level_get_component<Transform>(level,entity);
                return transform->GetLocalMatrix();
            }
        }
    }

    void spatial_update_system(level* level,float delta){
        auto entities_iter = level->entities.begin();
        auto entities_begin = level->entities.begin();
        while (entities_iter != level->entities.end()){
            uint32_t entity_idx = entities_iter-entities_begin;
            entity_id entity = CREATE_ID((*entities_iter),entity_idx);
            if(entity!=0 && level_has_components<Spatial3d>(level,entity)){

            }
            entities_iter++;
        }
    }

    std::vector<::entity_id> models(1024);
    std::vector<::entity_id> lights(124);
    std::vector<::entity_id> ui(128);

    void render_update_system(level* level,float delta){
        models.clear();
        lights.clear();
        ui.clear();

        auto entities_iter = level->entities.begin();
        auto entities_begin = level->entities.begin();

        ::entity_id main_camera = 0;
        while (entities_iter != level->entities.end()){
            uint32_t entity_idx = entities_iter-entities_begin;
            entity_id entity = CREATE_ID((*entities_iter),entity_idx);
            if(entity!=0 && level_has_components<Light,CastShadow>(level,entity)){
                lights.push_back(entity);
            }
            if(entity!=0 && level_has_components<Mesh,ReceiveShadow>(level,entity)){
                models.push_back(entity);
            }
            if(entity!=0 && level_has_components<Canvas>(level,entity)){
                ui.push_back(entity);
            }
            if(entity!=0 && level_has_components<MainCamera>(level,entity)){
                main_camera = entity;
            }
            entities_iter++;
        }

        render_shadow_phase(level,delta,models,lights);
        render_mesh_phase(level,delta,models,lights,main_camera);
        render_ui_phase(level,delta,ui);

        auto* frameState = level_get_share_resource<FrameState>(level);
        frameState->delta_time = delta;
        frameState->total_time += delta;
        frameState->total_count += 1;
    }

    void render_shadow_phase(level* level,float delta,vector<entity_id>& models,vector<entity_id>& lights){
        glm::mat4 shadowCubeTransforms[6] = {mat4{1.0}};
        auto right_left_face_view = [](vec3 position,float dir)->mat4 {
            return lookAt(position, position + vec3(dir,0.0,0.0), vec3(0.0,-1.0,0.0));
        };
        auto up_down_face_view = [](vec3 position,float dir)->mat4{
            return lookAt(position, position + vec3(0.0,dir,0.0), vec3(0.0,0.0,dir));
        };
        auto near_far_face_view = [](vec3 position,float dir)->mat4{
            return lookAt(position, position + vec3(0.0,0.0,dir), vec3(0.0,-1.0,0.0));
        };

        auto depth_shader = level_get_share_resource<DepthShader>(level);
        auto depth_cube_shader = level_get_share_resource<DepthCubeShader>(level);

        for (auto & light_entity:lights) {
            auto* light = level_get_component<Light>(level,light_entity);

            if(!light->has_shadow_map){
                continue;
            }
            glViewport(0, 0, light->shadow_map.width, light->shadow_map.height);
            glBindFramebuffer(GL_FRAMEBUFFER, light->shadow_map.fbo);
            glClear(GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
//            glEnable(GL_CULL_FACE);
//            glCullFace(GL_BACK);
//            glCullFace(GL_FRONT);

            uint32_t shaderProgram = depth_shader->id;

            if(light->type==Light::point){
                shaderProgram = depth_cube_shader->id;
                ShaderProgram::Bind(shaderProgram);

                shadowCubeTransforms[0] = light->p * right_left_face_view(light->position,1.0);//right
                shadowCubeTransforms[1] = light->p * right_left_face_view(light->position,-1.0f);//left
                shadowCubeTransforms[2] = light->p * up_down_face_view(light->position,1.0);//up
                shadowCubeTransforms[3] = light->p * up_down_face_view(light->position,-1.0);//down
                shadowCubeTransforms[4] = light->p * near_far_face_view(light->position,1.0);//near
                shadowCubeTransforms[5] = light->p * near_far_face_view(light->position,-1.0);//far

                for (int i = 0; i < 6; ++i) {
                    ShaderProgram::SetMat4(shaderProgram,"shadowMatrices["+ to_string(i)+"]", value_ptr(shadowCubeTransforms[i]));
                }
                ShaderProgram::SetVec3(shaderProgram,"lightPos", value_ptr(light->position));
                ShaderProgram::SetFloat(shaderProgram,"far_plane",light->far_plane);
            }else if(light->type==Light::spot||light->type==Light::directional){
                shaderProgram = depth_shader->id;
                ShaderProgram::Bind(shaderProgram);

                mat4 pv = light->p * light->v;
                ShaderProgram::SetMat4(shaderProgram, "PV", value_ptr(pv));
            }

            for (auto& model_entity:models) {
                auto mesh= level_get_component<Mesh>(level,model_entity);
                mat4 M = calcGlobalMatrix(level,model_entity);
                ShaderProgram::SetMat4(shaderProgram, "M", value_ptr(M));
                auto has_skin = level_has_components<Skin>(level,model_entity);
                ShaderProgram::SetInt(shaderProgram,"use_skin",has_skin);
                if(has_skin){
                    auto skin = level_get_component<Skin>(level,model_entity);
                    for (int i = 0; i < skin->joints_count; ++i) {
                        mat4 inverse_matrices = skin->inverse_bind_matrices[i];
                        mat4 joint_matrices = calcGlobalMatrix(level,skin->joints[i]);
                        joint_matrices = joint_matrices * inverse_matrices;
                        ShaderProgram::SetMat4(shaderProgram,"JointMat["+ to_string(i) + "]", value_ptr(joint_matrices));
                    }
                }

                for (auto& primitive:mesh->primitives) {
                    glBindVertexArray(primitive.vao);
                    if(primitive.indices_count==0){
                        glDrawArrays(GL_TRIANGLES,0,primitive.vertices_count);
                    }else{
                        glDrawElements(GL_TRIANGLES,primitive.indices_count,primitive.indices_type,nullptr);
                    }
                    glBindVertexArray(0);
                }
            }

            ShaderProgram::Unbind();

//            glCullFace(GL_BACK);
//            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }

    void render_mesh_phase(level* level,float delta,vector<entity_id>& models,vector<entity_id>& lights,entity_id camera){
        auto window = level_get_share_resource<Window>(level);
        auto vp = level_get_share_resource<VPMatrices>(level);
        auto pbrShader = level_get_share_resource<PBRShader>(level);
        auto* frameState = level_get_share_resource<FrameState>(level);

        glViewport(0,0,window->GetWidth(),window->GetHeight());
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        if(level_has_share_resource<RenderState>(level)){
            auto* renderState = level_get_share_resource<RenderState>(level);
            if(renderState->mode==RenderState::Wireframe){
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            if(renderState->mode==RenderState::Shader){
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
        }

        mat4 P = vp->project;
        mat4 V = vp->view;

        uint32_t shaderProgram = pbrShader->id;
        ShaderProgram::Bind(shaderProgram);
        ShaderProgram::SetMat4(shaderProgram, "P", value_ptr(P));
        ShaderProgram::SetMat4(shaderProgram, "V", value_ptr(V));
        ShaderProgram::SetFloat(shaderProgram,"time",frameState->total_time);
        vec3 camPos = level_get_component<Camera>(level,camera)->position;
        ShaderProgram::SetVec3(shaderProgram,"cameraPos", value_ptr(camPos));

        ShaderProgram::SetInt(shaderProgram,"albedo_texture",0);
        ShaderProgram::SetInt(shaderProgram,"metallic_roughness_texture",1);
        ShaderProgram::SetInt(shaderProgram,"occlusion_texture",2);
        ShaderProgram::SetInt(shaderProgram,"normal_texture",3);

        ShaderProgram::SetInt(shaderProgram,"light_num",lights.size());

        std::vector<Light*> render_lights;
        for(auto light_entity:lights){
            auto * light = level_get_component<Light>(level,light_entity);
            render_lights.push_back(light);
        }
        ShaderProgram::SetLight(shaderProgram,"lights",render_lights);

        int shadowMapOffset = 4;
        for (int i = 0; i < 5; ++i) {
            ShaderProgram::SetInt(shaderProgram,"shadowMap["+ to_string(i)+"]",shadowMapOffset+i);
            ShaderProgram::SetInt(shaderProgram,"shadowCubeMap["+ to_string(i)+"]",shadowMapOffset+5+i);
        }

        int cube_map_index = 0;
        int map_index = 0;
        for (auto light : render_lights) {
            if(light->has_shadow_map){
                if(light->type==Light::point){
                    int index = shadowMapOffset+5+cube_map_index;
                    glBindTextureUnit(index,light->shadow_map.texture);
                    light->shadow_map_index = cube_map_index;
                    cube_map_index++;
                }
                if(light->type==Light::directional||light->type==Light::spot){
                    int index = shadowMapOffset+map_index;
                    glBindTextureUnit(index,light->shadow_map.texture);
                    light->shadow_map_index = map_index;
                    mat4 pv = light->p * light->v;
                    ShaderProgram::SetMat4(shaderProgram, "LightPV["+ to_string(map_index)+"]", value_ptr(pv));
                    map_index++;
                }
            }
        }

        for (auto& model_entity:models) {
            auto mesh= level_get_component<Mesh>(level,model_entity);
            mat4 M = calcGlobalMatrix(level,model_entity);
            ShaderProgram::SetMat4(shaderProgram, "M", value_ptr(M));
            auto has_skin = level_has_components<Skin>(level,model_entity);
            ShaderProgram::SetInt(shaderProgram,"use_skin",has_skin);
            if(has_skin){
                auto skin = level_get_component<Skin>(level,model_entity);
                for (int i = 0; i < skin->joints_count; ++i) {
                    mat4 inverse_matrices = skin->inverse_bind_matrices[i];
                    mat4 joint_matrices = calcGlobalMatrix(level,skin->joints[i]);
                    joint_matrices = joint_matrices * inverse_matrices;
                    ShaderProgram::SetMat4(shaderProgram,"JointMat["+ to_string(i) + "]", value_ptr(joint_matrices));
                }
            }

            for (auto& primitive:mesh->primitives) {
                material_t mat = primitive.material;
                if(mat.has_albedo_texture){
                    glBindTextureUnit(0,mat.albedo_texture);
                }

                if(mat.has_metallic_roughness_texture){
                    glBindTextureUnit(1,mat.metallic_roughness_texture);
                }

                if(mat.has_occlusion_texture){
                    glBindTextureUnit(2,mat.occlusion_texture);
                }

                if(mat.has_normal_texture){
                    glBindTextureUnit(3,mat.normal_texture);
                }

                ShaderProgram::SetVec4(shaderProgram, "albedo_factor", value_ptr(mat.albedo_factor));
                ShaderProgram::SetInt(shaderProgram,"has_albedo_texture",mat.has_albedo_texture);

                ShaderProgram::SetFloat(shaderProgram, "metallic_factor", mat.metallic_factor);
                ShaderProgram::SetFloat(shaderProgram, "roughness_factor", mat.roughness_factor);
                ShaderProgram::SetInt(shaderProgram,"has_metallic_roughness_texture",mat.has_metallic_roughness_texture);

                ShaderProgram::SetFloat(shaderProgram,"ao_factor",mat.ao);
                ShaderProgram::SetInt(shaderProgram,"has_occlusion_texture",mat.has_occlusion_texture);
                ShaderProgram::SetDouble(shaderProgram,"occlusion_strength",mat.occlusion_strength);

                ShaderProgram::SetInt(shaderProgram,"has_normal_texture",mat.has_normal_texture);
                ShaderProgram::SetDouble(shaderProgram,"normal_scale",mat.normal_scale);

                glBindVertexArray(primitive.vao);
                if(primitive.indices_count==0){
                    glDrawArrays(GL_TRIANGLES,0,primitive.vertices_count);
                }else{
                    glDrawElements(GL_TRIANGLES,primitive.indices_count,primitive.indices_type,nullptr);
                }
                glBindVertexArray(0);
            }
        }

        ShaderProgram::Unbind();

        glDisable(GL_DEPTH_TEST);
    }

    void render_ui_phase(level* level,float delta,const vector<::entity_id>& items){
        auto window = level_get_share_resource<Window>(level);
        auto vp = level_get_share_resource<VPMatrices>(level);
        auto flatShader = level_get_share_resource<FlatShader>(level);
        auto* frameState = level_get_share_resource<FrameState>(level);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        ShaderProgram::Bind(flatShader->id);
        mat4 P = vp->ortho;
        vec3 color = {1.0,1.0,1.0};

        ShaderProgram::SetVec3(flatShader->id,"color", value_ptr(color));
        ShaderProgram::SetMat4(flatShader->id,"P", value_ptr(P));
        ShaderProgram::SetInt(flatShader->id,"texture0",0);
        for (auto item_entity:items) {
            auto canvas = level_get_component<Canvas>(level,item_entity);

            mat4 M{1.0f};
            M = translate(M,vec3(canvas->position,0.0f));
            M = scale(M,vec3(canvas->size,0.0f));
            ShaderProgram::SetMat4(flatShader->id,"M", value_ptr(M));
//        glActiveTexture(GL_TEXTURE0);
            glBindTextureUnit(0,canvas->texture);
            glBindVertexArray(canvas->vao);
            glDrawArrays(GL_TRIANGLES,0,6);
            glBindVertexArray(0);
        }
        ShaderProgram::Unbind();

        glDisable(GL_BLEND);
    }
}
