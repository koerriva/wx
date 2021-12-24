//
// Created by koerriva on 2021/12/21.
//

#include <iostream>
#include "log.h"
#include "renderer.h"
#include "font.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#define PI 3.1415926

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

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

    void Renderer::Render(const Window* window,const Camera* camera,const vector<Mesh>& meshList,const vector<Texture>& textures,ShaderProgram shaderProgram){
        glEnable(GL_DEPTH_TEST);
//        glEnable(GL_CULL_FACE);
//        glCullFace(GL_BACK);
        if(WIREFRAME_MODE){
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        if(SHADER_MODE){
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        glActiveTexture(GL_TEXTURE0);
        textures[0].Bind();

        ShaderProgram::Bind(shaderProgram);
        auto time = (float)Window::GetTimeInSecond();
        ShaderProgram::SetFloat(shaderProgram,"time",time);

        float aspect = window->GetAspect();
        glm::mat4 P,V(1.0f),M(1.0f);
        P = glm::perspective(glm::radians(60.f),aspect,.1f,99999.f);
        V = camera->GetViewMatrix();
        M = glm::rotate(M,time*0.1f,glm::vec3(0,1,0));
        M = glm::scale(M,glm::vec3(1.0f));

        ShaderProgram::SetMat4(shaderProgram, "P", value_ptr(P));
        ShaderProgram::SetMat4(shaderProgram, "V", value_ptr(V));
        ShaderProgram::SetMat4(shaderProgram, "M", value_ptr(M));

        vec4 base_color{1.0f};
        ShaderProgram::SetVec4(shaderProgram, "albedo", value_ptr(base_color));

        for (size_t i = 0; i < meshList.size(); i++)
        {
            auto& mesh = meshList[i];
            mesh.Draw();
        }

        ShaderProgram::Unbind();

        glDisable(GL_DEPTH_TEST);
    }

    void Renderer::Render(const Window* window,const Camera* camera,Terrain* terrain,ShaderProgram shaderProgram){
        glEnable(GL_DEPTH_TEST);
//        glEnable(GL_CULL_FACE);
//        glCullFace(GL_BACK);
        if(WIREFRAME_MODE){
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        if(SHADER_MODE){
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        float aspect = window->GetAspect();
        glm::mat4 P,V(1.0f),M(1.0f);
        P = glm::perspective(glm::radians(60.f),aspect,.1f,1000.f);
        V = camera->GetViewMatrix();
        M = glm::scale(M,glm::vec3(1.0f));

        ShaderProgram::Bind(shaderProgram);
        ShaderProgram::SetMat4(shaderProgram, "P", value_ptr(P));
        ShaderProgram::SetMat4(shaderProgram, "V", value_ptr(V));
        ShaderProgram::SetMat4(shaderProgram, "M", value_ptr(M));

        glm::vec4 base_color{1.0f};
        ShaderProgram::SetVec4(shaderProgram, "albedo", value_ptr(base_color));

        terrain->Draw();

        ShaderProgram::Unbind();

        glDisable(GL_DEPTH_TEST);
    }

    void Renderer::Render(const Window *window, vector<model_t>& models, vector<light_t>& lights, float delta) {
        for (auto & light:lights) {
            glViewport(0, 0, light.shadow_map.width, light.shadow_map.height);
            glBindFramebuffer(GL_FRAMEBUFFER, light.shadow_map.fbo);
            glClear(GL_DEPTH_BUFFER_BIT);

            float aspect = window->GetAspect();

            uint32_t shaderProgram = light.shadow_map.shader;
            ShaderProgram::Bind(shaderProgram);
            ShaderProgram::SetMat4(shaderProgram, "PV", value_ptr(light.pv));

            for (auto& model:models) {
                mat4 M = translate(mat4{1},model.transform.position)
                    * rotate(mat4{1},model.transform.rotation.x,vec3{1.,0.,0.})
                    * rotate(mat4{1},model.transform.rotation.y,vec3{0.,1.,0.})
                    * rotate(mat4{1},model.transform.rotation.z,vec3{0.,0.,1.})
                    * scale(mat4{1},model.transform.scale);

                ShaderProgram::SetMat4(shaderProgram, "M", value_ptr(M));

                for (int i = 0; i < model.mesh_count; ++i) {
                    mesh_t& mesh = model.meshes[i];

                    glBindVertexArray(mesh.vao);
                    glDrawElements(GL_TRIANGLES,mesh.indices_count,mesh.indices_type,nullptr);
                    glBindVertexArray(0);
                }
            }

            ShaderProgram::Unbind();

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }

    void Renderer::Render(const Window *window, const Camera *camera, vector<model_t>& models, vector<light_t>& lights, float delta) {
        glViewport(0,0,window->GetFrameBufferWidth(),window->GetFrameBufferHeight());
        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
//        glEnable(GL_CULL_FACE);
//        glCullFace(GL_BACK);
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

        for (auto& model:models) {
            M = translate(mat4{1},model.transform.position)
                    * rotate(mat4{1},model.transform.rotation.x,vec3{1.,0.,0.})
                    * rotate(mat4{1},model.transform.rotation.y,vec3{0.,1.,0.})
                    * rotate(mat4{1},model.transform.rotation.z,vec3{0.,0.,1.})
                    * scale(mat4{1},model.transform.scale);
            for (int i = 0; i < model.mesh_count; ++i) {
                mesh_t& mesh = model.meshes[i];
                material_t& mat = mesh.materials[0];

                uint32_t shaderProgram = mat.program_id;
                ShaderProgram::Bind(shaderProgram);

                for (int j = 0; j < 3; ++j) {
                    glActiveTexture(GL_TEXTURE0+i);
                }
                if(mat.has_albedo_texture){
                    glBindTexture(GL_TEXTURE_2D,mat.albedo_texture);
                }
                if(mat.has_metallic_roughness_texture){
                    glBindTexture(GL_TEXTURE_2D,mat.metallic_roughness_texture);
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
                ShaderProgram::SetMat4(shaderProgram, "LightPV", value_ptr(lights[0].pv));

                glBindTexture(GL_TEXTURE_2D,lights[0].shadow_map.texture);
                glBindVertexArray(mesh.vao);
                glDrawElements(GL_TRIANGLES,mesh.indices_count,mesh.indices_type,nullptr);
                glBindVertexArray(0);

                ShaderProgram::Unbind();
            }
        }

        glDisable(GL_DEPTH_TEST);

        this->frame_time += delta;
        this->frame_count++;
    }

    vector<model_t> Renderer::LoadModelFromGLTF(const char *filename) {
        vector<model_t> models;
        cgltf_options options{};
        cgltf_data* data = NULL;
//        cgltf_result result = cgltf_parse_file(&options, filename, &data);
        int len = 0;
        void* mdata = (void *) AssetsLoader::LoadRawData(filename, &len);
        cgltf_result result = cgltf_parse(&options,mdata,len,&data);
        if (result == cgltf_result_success)
        {
            /* TODO make awesome stuff */

            cgltf_load_buffers(&options,data,filename);

            WX_CORE_INFO("Success load : {}",filename);
        }else{
            WX_CORE_ERROR("Can't find file : {}",filename);
            return models;
        }

        if(data->nodes_count<1){
            return models;
        }

        WX_CORE_INFO("Upload Shader ..." );

        unordered_map<const char*,material_t> materials;

        uint32_t shader = ShaderProgram::LoadShader("pbr");
        for (int i = 0; i < data->materials_count; ++i) {
            cgltf_material* cmat =  &data->materials[i];
            material_t material{};
            WX_CORE_TRACE("Upload Material {}",cmat->name);
            cgltf_texture_view baseColorTexture = cmat->pbr_metallic_roughness.base_color_texture;
            if(baseColorTexture.texture){
                material.has_albedo_texture = 1;
                material.albedo_texture_index = baseColorTexture.texcoord;

                char * img_type = baseColorTexture.texture->image->mime_type;

                int data_size = baseColorTexture.texture->image->buffer_view->size;
                int data_offset = baseColorTexture.texture->image->buffer_view->offset;
                void * data = (uint8 *)(baseColorTexture.texture->image->buffer_view->buffer->data) + data_offset;
                int min_filter = baseColorTexture.texture->sampler->min_filter;
                int mag_filter = baseColorTexture.texture->sampler->mag_filter;
                int warp_s = baseColorTexture.texture->sampler->wrap_s;
                int warp_t = baseColorTexture.texture->sampler->wrap_t;
                material.albedo_texture = Texture::Load((unsigned char*)data,data_size,{min_filter,mag_filter},{warp_s,warp_t});
            }else{
                cgltf_float* baseColor = cmat->pbr_metallic_roughness.base_color_factor;
                material.albedo = {baseColor[0], baseColor[1], baseColor[2], baseColor[3]};
            }

            cgltf_texture_view metallicRoughnessTexture = cmat->pbr_metallic_roughness.metallic_roughness_texture;

            if(metallicRoughnessTexture.texture){
                material.has_metallic_roughness_texture = 1;
                material.metallic_roughness_texture_index = metallicRoughnessTexture.texcoord;

                char * img_type = metallicRoughnessTexture.texture->image->mime_type;

                int data_size = metallicRoughnessTexture.texture->image->buffer_view->size;
                int data_offset = metallicRoughnessTexture.texture->image->buffer_view->offset;
                void * data = (uint8 *)(metallicRoughnessTexture.texture->image->buffer_view->buffer->data) + data_offset;
                int min_filter = metallicRoughnessTexture.texture->sampler->min_filter;
                int mag_filter = metallicRoughnessTexture.texture->sampler->mag_filter;
                int warp_s = metallicRoughnessTexture.texture->sampler->wrap_s;
                int warp_t = metallicRoughnessTexture.texture->sampler->wrap_t;
                material.metallic_roughness_texture = Texture::Load((unsigned char*)data,data_size,{min_filter,mag_filter},{warp_s,warp_t});
            }else{
                material.metallic = cmat->pbr_metallic_roughness.metallic_factor;
                material.roughness = cmat->pbr_metallic_roughness.roughness_factor;
            }

            material.program_id = shader;

            materials[cmat->name] = material;
        }

        WX_CORE_INFO("Total model {}",data->nodes_count);
        for (int i = 0; i < data->nodes_count; ++i) {
            cgltf_node cnode = data->nodes[i];
            cgltf_mesh* cmesh = cnode.mesh;

            if(!cnode.mesh){
                continue;
            }

            model_t model;

            if(cnode.has_translation){
                model.transform.position = {cnode.translation[0],cnode.translation[1],cnode.translation[2]};
            }

            if(cnode.has_rotation){
                vec4 quatRotation{cnode.rotation[0],cnode.rotation[1],cnode.rotation[2],cnode.rotation[3]};
                vec3 eulerRotation = eulerAngles(quat(quatRotation));
                model.transform.rotation = eulerRotation;
            }

            if(cnode.has_scale){
                model.transform.scale = {cnode.scale[0],cnode.scale[1],cnode.scale[2]};
            }

            for (int j = 0; j < cmesh->primitives_count; ++j) {
                mesh_t mesh;

                std::cout << "Upload Mesh ..." << std::endl;

                glGenVertexArrays(1,&mesh.vao);
                glBindVertexArray(mesh.vao);

                uint32_t vbo = 0;


                std::cout << "Mesh : " << cmesh->name << std::endl;

                cgltf_primitive primitive = cmesh->primitives[j];

                cgltf_accessor* position_accessor = nullptr;
                cgltf_accessor* normal_accessor = nullptr;
                cgltf_accessor* texcoord_accessor = nullptr;
                cgltf_accessor* indices_accessor = primitive.indices;

                for (int k = 0; k < primitive.attributes_count; ++k) {
                    auto attr = primitive.attributes[k];
                    std::cout << "Attr L : " << attr.name << std::endl;
                    if(strcmp(attr.name,"POSITION")==0){
                        std::cout << "Attr : " << attr.name << std::endl;
                        position_accessor = attr.data;
                        continue;
                    }
                    if(strcmp(attr.name,"NORMAL")==0){
                        std::cout << "Attr : " << attr.name << std::endl;
                        normal_accessor = attr.data;
                        continue;
                    }
                    if(strcmp(attr.name,"TEXCOORD_0")==0){
                        std::cout << "Attr : " << attr.name << std::endl;
                        texcoord_accessor = attr.data;
                        continue;
                    }
                }

                int offset = position_accessor->buffer_view->offset;
                void* vertices_buffer = (uint8_t*)(position_accessor->buffer_view->buffer->data)+offset;
                int vertices_num = position_accessor->count;
                int vertices_size = position_accessor->buffer_view->size;

                offset = normal_accessor->buffer_view->offset;
                void* normal_buffer = (uint8_t*)(normal_accessor->buffer_view->buffer->data)+offset;
                int normal_num = normal_accessor->count;
                int normal_size = normal_accessor->buffer_view->size;

                void* texcoord0_buffer;
                int texcoord0_num;
                int texcoord0_size;
                if(texcoord_accessor){
                    offset = texcoord_accessor->buffer_view->offset;
                    texcoord0_buffer = (uint8_t*)(texcoord_accessor->buffer_view->buffer->data)+offset;
                    texcoord0_num = texcoord_accessor->count;
                    texcoord0_size = texcoord_accessor->buffer_view->size;
                }

                offset = indices_accessor->buffer_view->offset;
                void* indices_buffer = (uint8_t*)(indices_accessor->buffer_view->buffer->data)+offset;
                int indices_num = indices_accessor->count;
                int indices_size = indices_accessor->buffer_view->size;

                mesh.vertices_count = vertices_num;
                mesh.indices_count = indices_num;

                //vertices
                glGenBuffers(1,&vbo);
                glBindBuffer(GL_ARRAY_BUFFER,vbo);
                glBufferData(GL_ARRAY_BUFFER,vertices_size,vertices_buffer,GL_STATIC_DRAW);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);

                //normals
                glGenBuffers(1,&vbo);
                glBindBuffer(GL_ARRAY_BUFFER,vbo);
                glBufferData(GL_ARRAY_BUFFER,normal_size,normal_buffer,GL_STATIC_DRAW);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);

                //texcoords
                if(texcoord_accessor){
                    glGenBuffers(1,&vbo);
                    glBindBuffer(GL_ARRAY_BUFFER,vbo);
                    glBufferData(GL_ARRAY_BUFFER,texcoord0_size,texcoord0_buffer,GL_STATIC_DRAW);
                    glEnableVertexAttribArray(2);
                    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,2*sizeof(float),nullptr);
                }

                uint32_t ebo = 0 ;
                glGenBuffers(1,&ebo);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER,indices_size,indices_buffer,GL_STATIC_DRAW);

                mesh.indices_type = GL_UNSIGNED_SHORT;
                glBindVertexArray(0);

                if(primitive.material){
                    if(primitive.material->has_pbr_metallic_roughness){
                        cgltf_material* cmat = primitive.material;
                        WX_CORE_TRACE("Set Material {}",cmat->name);
                        mesh.materials[mesh.material_count] = materials[cmat->name];
                        mesh.material_count++;
                    }
                }else{
                    mesh.materials[mesh.material_count].albedo = {1.f, 0.3f, 0.3f, 1.0f};
                    mesh.materials[mesh.material_count].metallic = 0.f;
                    mesh.materials[mesh.material_count].roughness = 1.0f;
                    mesh.materials[mesh.material_count].program_id = shader;
                    mesh.material_count++;
                }

                model.meshes[model.mesh_count++] = mesh;
            }
            models.push_back(model);
        }

        WX_CORE_INFO("Load Complete!");
        cgltf_free(data);
        return models;
    }

    //Mesh
    Mesh::Mesh(vector<float> &vertices,vector<unsigned>& indices,vector<float> &normals,vector<float> &texCoords,vector<float>& colors) {
        this->vertices = vertices;
        this->indices = indices;
        this->normals = normals;
        this->texCoords = texCoords;

        glGenVertexArrays(1,&vao);
        glBindVertexArray(vao);

        cout << "vertices :" << vertices.size() << endl;
        unsigned int vbo;

        //vertices
        glGenBuffers(1,&vbo);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(float),vertices.data(),GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);
        vbos.push_back(vbo);

        //normals
        glGenBuffers(1,&vbo);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,normals.size()*sizeof(float),normals.data(),GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);
        vbos.push_back(vbo);

        //texcoords
        glGenBuffers(1,&vbo);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,texCoords.size()*sizeof(float),texCoords.data(),GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,2*sizeof(float),nullptr);
        vbos.push_back(vbo);

        if(!colors.empty()){
            //colors
            glGenBuffers(1,&vbo);
            glBindBuffer(GL_ARRAY_BUFFER,vbo);
            glBufferData(GL_ARRAY_BUFFER,colors.size()*sizeof(float),colors.data(),GL_STATIC_DRAW);
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);
            vbos.push_back(vbo);
        }

        glGenBuffers(1,&ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,indices.size()*sizeof(unsigned),indices.data(),GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    Mesh::~Mesh(){
        cout << "Drop Mesh" << endl;
    }

    void Mesh::Draw() const {
        glBindVertexArray(vao);
//        glDrawArrays(GL_TRIANGLES,0,3);
        glDrawElements(GL_TRIANGLES,indices.size(),GL_UNSIGNED_INT,nullptr);
        glBindVertexArray(0);
    }

    void Mesh::Cleanup() const {
        cout << "Clean Mesh " << vao << endl;
        glDeleteVertexArrays(1,&vao);
        glDeleteBuffers(vbos.size(),vbos.data());
    }

    Mesh Mesh::Sphere(float r,int sectors,int stacks) {
        vector<float> vertices;
        vector<float> colors;
        vector<unsigned int> indices;
        vector<float> normals;
        vector<float> texCoords;

        float sectorStep = float(2.*PI) / float(sectors);//圆周等分
        auto stackStep = float(PI / stacks); //半圆等分
        float len = 1.0f/float(r);

        ofstream file("log.txt");
        for (int i = 0; i <= stacks; ++i) {
            auto stackAngle = float(PI/2 - float(i)*stackStep); //垂直角
            float y = r*sinf(stackAngle);
            float xz = r*cosf(stackAngle);

            for (int j = 0; j <= sectors; ++j) {
                float sectorAngle = float(j)*sectorStep; //水平角
                float x = xz*cosf(sectorAngle);
                float z = xz*sinf(sectorAngle);

                //顶点坐标
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);

                vec3 color(1.0,1.0,1.0);
                colors.push_back(color.r);
                colors.push_back(color.g);
                colors.push_back(color.b);

                //贴图坐标
                float s = float(j)/float(sectors);
                float t = float(i)/float(stacks);
                texCoords.push_back(s);
                texCoords.push_back(t);

                //法线
                normals.push_back(x*len);
                normals.push_back(y*len);
                normals.push_back(z*len);
            }
        }

        for (int v = 0; v < stacks; ++v) {
            int v0 = v*(sectors+1);//起点
            int v1 = v0+sectors+1;//终点
            for (int h = 0; h < sectors; ++h,++v0,++v1) {
                if(v!=0){
                    indices.push_back(v0);
                    indices.push_back(v1);
                    indices.push_back(v0+1);
                }
                if(v!=(stacks-1)){
                    indices.push_back(v0+1);
                    indices.push_back(v1);
                    indices.push_back(v1+1);
                }
            }
        }

        return Mesh(vertices,indices,normals,texCoords,colors);
    }

    void Mesh::DumpPNGFile(int width, int height, vector<float> &colors) {
        int pos=0,comps=3;
        vector<unsigned char> buffer;
        buffer.resize(width*height*comps);

        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                buffer[pos] = static_cast<int>(colors[pos]*255.99);
                pos++;
                buffer[pos] = static_cast<int>(colors[pos]*255.99);
                pos++;
                buffer[pos] = static_cast<int>(colors[pos]*255.99);
                pos++;
            }
        }

        stbi_write_png("terrain.png",width,height,comps,buffer.data(),0);
    }

    /**
     * texture
     * @param buffer
     * @param len
     */
    Texture::Texture(const unsigned char *buffer, int len) {
        this->buffer = buffer;
        this->len = len;
        unsigned char* data = stbi_load_from_memory(buffer,len,&width,&height,&comp,0);
        WX_CORE_INFO("Image Info width={}, height={}, channels={}",width,height,comp);
        glGenTextures(1,&texture);
        //为当前绑定的纹理对象设置环绕、过滤方式
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //生成纹理
        glBindTexture(GL_TEXTURE_2D,texture);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }

    void Texture::Bind() const {
        glBindTexture(GL_TEXTURE_2D,texture);
    }

    void Texture::Cleanup() {
        cout << "Cleanup Texture" << endl;
        glDeleteTextures(1,&texture);
    }

    uint32_t Texture::Load(const unsigned char *buffer, int len,ivec2 filter,ivec2 warp) {
        int width,height,comp;
        uint32_t texture;
        unsigned char* data = stbi_load_from_memory(buffer,len,&width,&height,&comp,0);
        WX_CORE_INFO("Image Info width={}, height={}, channels={}",width,height,comp);
        glGenTextures(1,&texture);
        //为当前绑定的纹理对象设置环绕、过滤方式
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, warp.x!=0?warp.x:GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, warp.y!=0?warp.y:GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter.x!=0?filter.x:GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter.y!=0?filter.y:GL_LINEAR);
        //生成纹理
        glBindTexture(GL_TEXTURE_2D,texture);
        if(comp==4){
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,data);
        }else if(comp==3){
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,data);
        }else{
            WX_CORE_ERROR("Image Format Unsupported");
        }
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
        return texture;
    }

    shadow_map_t Texture::LoadDepthMap(uint32_t width,uint32_t height) {
        GLuint depthMapFBO;
        glGenFramebuffers(1, &depthMapFBO);

        GLuint depthMap;
        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                     width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        //只关心深度缓冲
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        shadow_map_t shadowMap;
        shadowMap.fbo = depthMapFBO;
        shadowMap.texture = depthMap;
        shadowMap.width = width;
        shadowMap.height = height;
        return shadowMap;
    }

    /**
     * ShaderProgram
     * @param type
     * @return
     */

    unordered_map<string,int> ShaderProgram::uniforms;
    unsigned int ShaderProgram::CreateShader(GLuint type,const char* source) {
        unsigned int shader = glCreateShader(type);
        if (type==GL_VERTEX_SHADER){
            glShaderSource(shader,1,&source, nullptr);

        }else if(type==GL_FRAGMENT_SHADER){
            glShaderSource(shader,1,&source, nullptr);
        }else{
            WX_CORE_CRITICAL("Unsupported Shader type {}",type);
            exit(-1001);
        }
        glCompileShader(shader);
        int success;
        char info[512];
        glGetShaderiv(shader,GL_COMPILE_STATUS,&success);
        if(!success){
            glGetShaderInfoLog(shader,512, nullptr,info);
            WX_CORE_CRITICAL(info);
            exit(-1002);
        }
        return shader;
    }

    unsigned int ShaderProgram::LoadShader(const char* name) {
        unsigned int program = glCreateProgram();
        const char* vertShaderSource = AssetsLoader::LoadShader(name,ShaderType::VERTEX_SHADER);
        const char* fragShaderSource = AssetsLoader::LoadShader(name,ShaderType::FRAGMENT_SHADER);
        unsigned vertexShader = CreateShader(GL_VERTEX_SHADER,vertShaderSource);
        unsigned fragmentShader = CreateShader(GL_FRAGMENT_SHADER,fragShaderSource);
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);
        int success;
        char info[512];
        glGetProgramiv(program,GL_LINK_STATUS, &success);
        if(!success){
            glGetProgramInfoLog(program,512, nullptr,info);
            WX_CORE_CRITICAL(info);
            exit(-1002);
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return program;
    }

    void ShaderProgram::Bind(uint32_t pid) {
        glUseProgram(pid);
    }

    void ShaderProgram::Unbind() {
        glUseProgram(0);
    }

    void ShaderProgram::Cleanup(uint32_t pid) {
        cout << "Clean Program " << pid << endl;
        glDeleteProgram(pid);
    }

    void ShaderProgram::SetFloat(uint32_t pid,const string& _name, float value) {
        int location = 0;
        string name = to_string(pid) + "_" + _name;
        if(uniforms.count(name)==0){
            cout << "Find Uniform : " << name << endl;
            location = glGetUniformLocation(pid,_name.c_str());
            cout << "Uniform[" << name << "] Location=" << location << endl;
            uniforms[name]=location;
        }else{
            location = uniforms[name];
        }
        glUniform1f(location,value);
    }
    void ShaderProgram::SetMat4(uint32_t pid,const string& _name, float *value) {
        int location = 0;
        string name = to_string(pid) + "_" + _name;
        if(uniforms.count(name)==0){
            cout << "Find Uniform : " << name << endl;
            location = glGetUniformLocation(pid,_name.c_str());
            cout << "Uniform[" << name << "] Location=" << location << endl;
            uniforms[name]=location;
        }else{
            location = uniforms[name];
        }
        glUniformMatrix4fv(location,1,GL_FALSE,value);
    }
    void ShaderProgram::SetVec4(uint32_t pid,const string& _name, float *value) {
        int location = 0;
        string name = to_string(pid) + "_" + _name;
        if(uniforms.count(name)==0){
            cout << "Find Uniform : " << name << endl;
            location = glGetUniformLocation(pid,_name.c_str());
            cout << "Uniform[" << name << "] Location=" << location << endl;
            uniforms[name]=location;
        }else{
            location = uniforms[name];
        }
        glUniform4fv(location,1,value);
    }
    void ShaderProgram::SetVec3(uint32_t pid,const string& _name, float *value) {
        int location = 0;
        string name = to_string(pid) + "_" + _name;
        if(uniforms.count(name)==0){
            cout << "Find Uniform : " << name << endl;
            location = glGetUniformLocation(pid,_name.c_str());
            cout << "Uniform[" << name << "] Location=" << location << endl;
            uniforms[name]=location;
        }else{
            location = uniforms[name];
        }
        glUniform3fv(location,1,value);
    }

    void ShaderProgram::SetInt(uint32_t pid, const string& _name, int value) {
        int location = 0;
        string name = to_string(pid) + "_" + _name;
        if(uniforms.count(name)==0){
            cout << "Find Uniform : " << name << endl;
            location = glGetUniformLocation(pid,_name.c_str());
            cout << "Uniform[" << name << "] Location=" << location << endl;
            uniforms[name]=location;
        }else{
            location = uniforms[name];
        }
        glUniform1i(location,value);
    }

    void ShaderProgram::SetAttenuation(uint32_t pid, const string& _name, attenuation_t value){
        ShaderProgram::SetFloat(pid,_name+".constant",value.constant);
        ShaderProgram::SetFloat(pid,_name+".exponent",value.exponent);
        ShaderProgram::SetFloat(pid,_name+".linear",value.linear);
    }

    void ShaderProgram::SetLight(uint32_t pid, const string& _name, vector<light_t> &lights) {
        for (int i = 0; i < lights.size(); ++i) {
            auto& light = lights[i];
            string name = "lights["+ to_string(i)+"]";
            ShaderProgram::SetInt(pid,name+".type",light.type);
            ShaderProgram::SetVec3(pid,name+".color", value_ptr(light.color));
            ShaderProgram::SetVec3(pid,name+".position", value_ptr(light.position));
            ShaderProgram::SetVec3(pid,name+".direction", value_ptr(light.direction));
            ShaderProgram::SetFloat(pid,name+".intensity",light.intensity);
            ShaderProgram::SetFloat(pid,name+".cutoff",light.cutoff);
            ShaderProgram::SetInt(pid,name+".has_shadow_map",light.has_shadow_map);
            ShaderProgram::SetAttenuation(pid,name+".att",light.attenuation);
        }
    }

    QuadTreeNode *Terrain::CreateNewChunk(size_t depth, vec3 center, float bound, QuadTreeNode *parent) {
//        WX_CORE_INFO("Create QuadTreeNode ({},{},{}),{:.4f}",center.x,center.y,center.z,bound);
        QuadTreeNode node;
        node.depth = depth;
        node.center = center;
        node.bound = bound;
        node.parent = parent;
//        WX_CORE_INFO("Set Node[{}] Bound {:.4f}",node.id,bound);
        vec3 top_left = center+vec3(-bound/2,0.f,-bound/2);
        vec3 top_right = center+vec3(bound/2,0.f,-bound/2);
        vec3 bot_left = center+vec3(-bound/2,0.f,bound/2);
        vec3 bot_right = center+vec3(bound/2,0.f,bound/2);
        vector<vec3> buffer;
        buffer.push_back(top_left);
        buffer.push_back(top_right);
        buffer.push_back(bot_right);
        buffer.push_back(bot_left);
        auto *ptr = reinterpret_cast<float *>(buffer.data());
        for (int i = 0; i < 12; ++i) {
            node.patch.points[i] = *(ptr + i);
        }
        node.patch.Upload();

        node.id = next_id++;
        chunks[node.id]=node;
        return chunks+node.id;
    }

    void Terrain::SplitChunk(QuadTreeNode *parent) {
        float childBound = parent->bound/2;
        vec3 NW_OFFSET(-childBound/2,0.f,-childBound/2);
        vec3 NE_OFFSET(childBound/2,0.f,-childBound/2);
        vec3 SW_OFFSET(-childBound/2,0.f,childBound/2);
        vec3 SE_OFFSET(childBound/2,0.f,childBound/2);
        parent->children[0] = CreateNewChunk(parent->depth+1,parent->center+NW_OFFSET,childBound,parent);
        parent->children[1] = CreateNewChunk(parent->depth+1,parent->center+NE_OFFSET,childBound,parent);
        parent->children[2] = CreateNewChunk(parent->depth+1,parent->center+SW_OFFSET,childBound,parent);
        parent->children[3] = CreateNewChunk(parent->depth+1,parent->center+SE_OFFSET,childBound,parent);
        parent->children_count=4;
    }

    void Terrain::SplitChunk(QuadTreeNode *parent, int depth) {
        if(parent->depth<depth){
            if(parent->children_count==0){
                SplitChunk(parent);
            }else{
                for (int i = 0; i < parent->children_count; ++i) {
                    SplitChunk(parent->children[i],depth);
                }
            }
        }
    }

    tuple<float,vec3> TerrainFaceColor[]={
            make_tuple(1.0000f,vec3(1.0f,1.0f,1.0f)),//snow
            make_tuple(0.7500f,vec3(128.f/255.99f,128.f/255.99f,128.f/255.99f)),//rock
            make_tuple(0.3750f,vec3(224.f/255.99f,224.f/255.99f,0.f)),//dirt
            make_tuple(0.1250f,vec3(32.f/255.99f,160.f/255.99f,0.f)),//grass
            make_tuple(0.0625f,vec3(240.f/255.99f,240.f/255.99f,64.f)),//sand
            make_tuple(0.0000f,vec3(0.f,128.f/255.99f,1.f)),//shore
            make_tuple(-0.2500f,vec3(0.f,0.f,1.f)),//shallow water
            make_tuple(-1.000f,vec3(0.f,0.f,128.f/255.f)),//deep water
    };

    Terrain::Terrain() {
        cout << "QuadTreeNode Size:" << sizeof(QuadTreeNode) << endl;
        vec3 origin(0.f,0.f,0.f);
        float bound = 10.f;
        vec3 up_offset(0,0,-bound),down_offset(0,0,bound),left_offset(-bound,0,0),right_offset(bound,0,0);

        int init_depth=1;
        auto root = CreateNewChunk(init_depth,origin,bound, nullptr);
        root->neighbors[0] = CreateNewChunk(init_depth,origin+up_offset,bound,nullptr);
        root->neighbors[1] = CreateNewChunk(init_depth,origin+down_offset,bound,nullptr);
        root->neighbors[2] = CreateNewChunk(init_depth,origin+left_offset,bound,nullptr);
        root->neighbors[3] = CreateNewChunk(init_depth,origin+right_offset,bound,nullptr);
        root->neighbors_count = 4;

        target_chunk = root;
        target_depth = init_depth;

        WX_CORE_INFO("Root Node[{}] Depth[{}] {:.4f}",root->id,root->depth,root->bound);
    }

    void Patch::Upload() {
        unsigned int vao,vbo,ebo;
        glGenVertexArrays(1,&vao);
        glBindVertexArray(vao);

        glGenBuffers(1,&vbo);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,sizeof(this->points),this->points,GL_STATIC_DRAW);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float), nullptr);
        glEnableVertexAttribArray(0);

        glGenBuffers(1,&ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(this->indices),this->indices,GL_STATIC_DRAW);

        glBindVertexArray(0);

        this->vao = vao;
        this->vbo = vbo;
        this->ebo = ebo;
        this->loaded = true;
    }

    void Patch::Draw() {
        if(this->loaded){
            glBindVertexArray(this->vao);
            glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
        }
    }

    /**
     * Debug
     */
    struct Character{
        bool cached=false;
        GLuint texture;
        ivec2 size;
        ivec2 bearing;
        GLuint advance;
    };

    Debug::Debug() {
        shaderProgram = ShaderProgram::LoadShader("font");
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,4*sizeof(float),nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void Debug::PrintScreen(vec2 pos, const char* text, vec3 color) {
        glEnable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        ShaderProgram::Bind(shaderProgram);
        ShaderProgram::SetVec3(shaderProgram,"color", value_ptr(color));
        mat4 P = ortho(0.f,1280.f,720.f,0.f);
        ShaderProgram::SetMat4(shaderProgram, "P", value_ptr(P));
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);

        std::string chars = std::string(text);
        for (auto it=chars.begin();it!=chars.end();++it) {
            Character c = Font::GetChar(*it);
            float x = pos.x+c.bearing.x*1.0f;
            float y = pos.y+Font::PIXEL_SIZE+float((c.size.y-c.bearing.y))*1.0f;
            auto w = float(c.size.x);
            auto h = float(c.size.y);

            //6个顶点
            vertices ={
                    x,y-h,0.0,0.0,
                    x,y,0.0,1.0,
                    x+w,y,1.0,1.0,

                    x,y-h,0.0,0.0,
                    x+w,y,1.0,1.0,
                    x+w,y-h,1.0,0.0,
            };

            pos.x += c.advance>>6;

            glBindTexture(GL_TEXTURE_2D,c.texture);
            glBindBuffer(GL_ARRAY_BUFFER,VBO);
            glBufferSubData(GL_ARRAY_BUFFER,0,vertices.size()*sizeof(float),vertices.data());
            glDrawArrays(GL_TRIANGLES,0,6);
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D,0);

        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
    }
}
