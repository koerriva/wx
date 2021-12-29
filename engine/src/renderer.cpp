//
// Created by koerriva on 2021/12/21.
//

#include <iostream>
#include "log.h"
#include "renderer.h"
#include "font.h"

GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
        WX_CORE_CRITICAL("{} | {} ({})",error,file,line);
        exit(-10002);
    }
    return errorCode;
}

namespace wx {
    Renderer::Renderer(/* args */)
    = default;

    Renderer::~Renderer()
    {
        std::cout << "Drop Renderer" << std::endl;
    }

    void Renderer::Init(){
        glClearColor(0.f,0.f,0.f,1.0f);
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
    }

    void Renderer::SetWireframeMode(){
        WIREFRAME_MODE = true;
        SHADER_MODE = false;
    }

    void Renderer::SetShaderMode() {
        WIREFRAME_MODE = false;
        SHADER_MODE = true;
    }

    void Renderer::SetToLightView(light_t* light) {
        this->view_light = light;
        this->is_light_view = light != nullptr;
    }

    glm::mat4 shadowCubeTransforms[6] = {mat4{1.0}};
    void Renderer::Render(const Window *window, vector<model_t>& models, vector<light_t>& lights,float delta) {

        auto right_left_face_view = [](vec3 position,float dir)->mat4 {
            return lookAt(position, position + vec3(dir,0.0,0.0), vec3(0.0,-1.0,0.0));
        };
        auto up_down_face_view = [](vec3 position,float dir)->mat4{
            return lookAt(position, position + vec3(0.0,dir,0.0), vec3(0.0,0.0,dir));
        };
        auto near_far_face_view = [](vec3 position,float dir)->mat4{
            return lookAt(position, position + vec3(0.0,0.0,dir), vec3(0.0,-1.0,0.0));
        };

        for (auto & light:lights) {
            if(!light.has_shadow_map){
                continue;
            }
            glViewport(0, 0, light.shadow_map.width, light.shadow_map.height);
            glBindFramebuffer(GL_FRAMEBUFFER, light.shadow_map.fbo);
            glClear(GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
//            glEnable(GL_CULL_FACE);
//            glCullFace(GL_BACK);
//            glCullFace(GL_FRONT);

            uint32_t shaderProgram = light.shadow_map.shader;
            ShaderProgram::Bind(shaderProgram);

            if(light.type==point){
                shadowCubeTransforms[0] = light.p * right_left_face_view(light.position,1.0);//right
                shadowCubeTransforms[1] = light.p * right_left_face_view(light.position,-1.0f);//left
                shadowCubeTransforms[2] = light.p * up_down_face_view(light.position,1.0);//up
                shadowCubeTransforms[3] = light.p * up_down_face_view(light.position,-1.0);//down
                shadowCubeTransforms[4] = light.p * near_far_face_view(light.position,1.0);//near
                shadowCubeTransforms[5] = light.p * near_far_face_view(light.position,-1.0);//far

                for (int i = 0; i < 6; ++i) {
                    ShaderProgram::SetMat4(shaderProgram,"shadowMatrices["+ to_string(i)+"]", value_ptr(shadowCubeTransforms[i]));
                }
                ShaderProgram::SetVec3(shaderProgram,"lightPos", value_ptr(light.position));
                ShaderProgram::SetFloat(shaderProgram,"far_plane",light.far_plane);
            }

            if(light.type==spot||light.type==directional){
                mat4 pv = light.p * light.v;
                ShaderProgram::SetMat4(shaderProgram, "PV", value_ptr(pv));
            }

            for (auto& model:models) {
                mat4 M = translate(mat4{1},model.transform.position)
                         * rotate(mat4{1},model.transform.rotation.x,vec3{1.,0.,0.})
                         * rotate(mat4{1},model.transform.rotation.y,vec3{0.,1.,0.})
                         * rotate(mat4{1},model.transform.rotation.z,vec3{0.,0.,1.})
                         * scale(mat4{1},model.transform.scale);

                ShaderProgram::SetMat4(shaderProgram, "M", value_ptr(M));
                ShaderProgram::SetInt(shaderProgram,"use_skin",model.has_skin);
                if(model.has_skin==1){
                    for (int i = 0; i < model.skin.bind_mat_count; ++i) {
                        ShaderProgram::SetMat4(shaderProgram,"JointMat["+ to_string(i) + "]", value_ptr(model.skin.bind_mat[i]));
                    }
                }

                for (int i = 0; i < model.mesh_count; ++i) {
                    mesh_t& mesh = model.meshes[i];
                    glBindVertexArray(mesh.vao);
                    glDrawElements(GL_TRIANGLES,mesh.indices_count,mesh.indices_type,nullptr);
                    glBindVertexArray(0);
                }
            }

            ShaderProgram::Unbind();

//            glCullFace(GL_BACK);
//            glDisable(GL_CULL_FACE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }

    void Renderer::Render(const Window *window, const Camera *camera, vector<model_t>& models, vector<light_t>& lights,canvas_t canvas, float delta) {
        glViewport(0,0,window->GetFrameBufferWidth(),window->GetFrameBufferHeight());
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        if(WIREFRAME_MODE){
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        if(SHADER_MODE){
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        float aspect = window->GetAspect();
        glm::mat4 P(1.0f),V(1.0f),M(1.0f);
        P = glm::perspective(glm::radians(60.f),aspect,.1f,1000.f);
        V = camera->GetViewMatrix();

        if(is_light_view){
            P = view_light->p;
            V = view_light->v;
        }

        for (auto& model:models) {
            M = translate(mat4{1},model.transform.position)
                    * rotate(mat4{1},model.transform.rotation.x,vec3{1.,0.,0.})
                    * rotate(mat4{1},model.transform.rotation.y,vec3{0.,1.,0.})
                    * rotate(mat4{1},model.transform.rotation.z,vec3{0.,0.,1.})
                    * scale(mat4{1},model.transform.scale);

            uint32_t shaderProgram = model.material.program_id;
            ShaderProgram::Bind(shaderProgram);
            ShaderProgram::SetInt(shaderProgram,"albedo_texture",0);
            ShaderProgram::SetInt(shaderProgram,"metallic_roughness_texture",1);

            for (int i = 0; i < 5; ++i) {
                ShaderProgram::SetInt(shaderProgram,"shadowMap["+ to_string(i)+"]",2+i);
                ShaderProgram::SetInt(shaderProgram,"shadowCubeMap["+ to_string(i)+"]",2+5+i);
            }

            int cube_map_index = 0;
            int map_index = 0;
            for (auto & light : lights) {
                if(light.has_shadow_map){
                    if(light.type==point){
                        int index = 2+5+cube_map_index;
                        glBindTextureUnit(index,light.shadow_map.texture);
                        light.shadow_map_index = cube_map_index;
                        cube_map_index++;
                    }
                    if(light.type==directional||light.type==spot){
                        int index = 2+map_index;
                        glBindTextureUnit(index,light.shadow_map.texture);
                        light.shadow_map_index = map_index;
                        mat4 pv = light.p * light.v;
                        ShaderProgram::SetMat4(shaderProgram, "LightPV["+ to_string(map_index)+"]", value_ptr(pv));
                        map_index++;
                    }
                }
            }

            for (int i = 0; i < model.mesh_count; ++i) {
                mesh_t& mesh = model.meshes[i];
                material_instance_t& mat = mesh.materials[0];
//
//                glActiveTexture(GL_TEXTURE0);
//                glBindTexture(GL_TEXTURE_2D,mat.albedo_texture);
//                glActiveTexture(GL_TEXTURE0+1);
//                glBindTexture(GL_TEXTURE_2D,mat.metallic_roughness_texture);
//                glActiveTexture(GL_TEXTURE0+2);
//                glBindTexture(GL_TEXTURE_2D,lights[0].shadow_map.texture);

                if(mat.has_albedo_texture){
                    glBindTextureUnit(0,mat.albedo_texture);
                }

                if(mat.has_metallic_roughness_texture){
                    glBindTextureUnit(1,mat.metallic_roughness_texture);
                }

                ShaderProgram::SetInt(shaderProgram,"use_skin",model.has_skin);
                if(model.has_skin==1){
                    for (int j = 0; j < model.skin.bind_mat_count; ++j) {
                        ShaderProgram::SetMat4(shaderProgram,"JointMat["+ to_string(j) + "]", value_ptr(model.skin.bind_mat[j]));
                    }
                }

                ShaderProgram::SetMat4(shaderProgram, "P", value_ptr(P));
                ShaderProgram::SetMat4(shaderProgram, "V", value_ptr(V));
                ShaderProgram::SetMat4(shaderProgram, "M", value_ptr(M));
                ShaderProgram::SetFloat(shaderProgram,"time",frame_time);
                vec3 camPos = camera->Position();
                ShaderProgram::SetVec3(shaderProgram,"cameraPos", value_ptr(camPos));

                ShaderProgram::SetVec4(shaderProgram, "albedo", value_ptr(mat.albedo));
                ShaderProgram::SetInt(shaderProgram,"has_albedo_texture",mat.has_albedo_texture);

                ShaderProgram::SetFloat(shaderProgram, "metallic", mat.metallic);
                ShaderProgram::SetFloat(shaderProgram, "roughness", mat.roughness);
                ShaderProgram::SetFloat(shaderProgram,"ao",mat.ao);
                ShaderProgram::SetInt(shaderProgram,"has_metallic_roughness_texture",mat.has_metallic_roughness_texture);

                ShaderProgram::SetInt(shaderProgram,"light_num",lights.size());
                ShaderProgram::SetLight(shaderProgram,"lights",lights);

                glBindVertexArray(mesh.vao);
                glDrawElements(GL_TRIANGLES,mesh.indices_count,mesh.indices_type,nullptr);
                glBindVertexArray(0);

            }

            ShaderProgram::Unbind();
        }

//        glEnable(GL_BLEND);
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        ShaderProgram::Bind(canvas.shader);
        mat4 uP = ortho(0.f,window->GetWidth()*1.0f,window->GetHeight()*1.0f,0.f);
        vec3 color = {1.0,1.0,1.0};
        mat4 uM{1.0f};
        uM = translate(uM,vec3(canvas.position,0.0f));
        uM = scale(uM,vec3(canvas.size,0.0f));
        ShaderProgram::SetVec3(canvas.shader,"color", value_ptr(color));
        ShaderProgram::SetMat4(canvas.shader,"P", value_ptr(uP));
        ShaderProgram::SetMat4(canvas.shader,"M", value_ptr(uM));
        ShaderProgram::SetInt(canvas.shader,"texture0",0);
//        glActiveTexture(GL_TEXTURE0);
        glBindTextureUnit(0,canvas.texture);
        glBindVertexArray(canvas.vao);
        glDrawArrays(GL_TRIANGLES,0,6);
        glBindVertexArray(0);
        ShaderProgram::Unbind();

        this->frame_time += delta;
        this->frame_count++;
    }
}
