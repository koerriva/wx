//
// Created by koerriva on 2022/1/8.
//
#include <iostream>
#include <unordered_map>
#include "renderer.h"
#include "glm/ext.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glad/glad.h"

#define TINYGLTF_USE_CPP14
#define TINYGLTF_USE_RAPIDJSON
#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"

namespace wx {
    Mesh* get_model_component(level* scene,tinygltf::Model* cmodel,::entity_id entity,int mesh_idx);
    Skin* get_skin_component(level* scene,tinygltf::Model* cmodel,::entity_id entity,int skin_idx,std::unordered_map<int,::entity_id>& nodes);
    ::entity_id get_node(level* scene,tinygltf::Model* cmodel,::entity_id parent,int node_idx,std::unordered_map<int,::entity_id>& nodes);

    Mesh* get_model_component(level* scene,tinygltf::Model* cmodel,::entity_id entity,int mesh_idx){
        using namespace tinygltf;
        tinygltf::Mesh& cmesh = cmodel->meshes[mesh_idx];
        std::cout << "Mesh : " << cmesh.name << std::endl;

        auto model = level_add_component(scene,entity,Mesh{});
        model->name = cmesh.name;

        for (auto& primitive:cmesh.primitives) {
            Mesh::primitive_t mesh;
            std::cout << "Upload Primitive " << std::endl;

            glGenVertexArrays(1,&mesh.vao);
            glBindVertexArray(mesh.vao);

            uint32_t vbo = 0;

            Accessor* position_accessor = nullptr;
            Accessor* normal_accessor = nullptr;
            Accessor* texcoord_accessor = nullptr;
            Accessor* joint_accessor = nullptr;
            Accessor* weight_accessor = nullptr;
            Accessor* indices_accessor = nullptr;

            for (auto& attr:primitive.attributes) {
                if(attr.first=="POSITION"){
                    position_accessor = &cmodel->accessors[attr.second];
                    continue;
                }
                if(attr.first=="NORMAL"){
                    normal_accessor = &cmodel->accessors[attr.second];
                    continue;
                }
                if(attr.first=="TEXCOORD_0"){
                    texcoord_accessor = &cmodel->accessors[attr.second];
                    continue;
                }
                if(attr.first=="JOINTS_0"){
                    joint_accessor = &cmodel->accessors[attr.second];
                    continue;
                }
                if(attr.first=="WEIGHTS_0"){
                    weight_accessor = &cmodel->accessors[attr.second];
                    continue;
                }
            }

            if(position_accessor){
                BufferView* bufferView = &cmodel->bufferViews[position_accessor->bufferView];
                Buffer* buffer = &cmodel->buffers[bufferView->buffer];

                int offset = position_accessor->byteOffset + bufferView->byteOffset;
                int data_count = position_accessor->count;
                std::cout << "vertices : " << data_count << std::endl;
                mesh.vertices_count = data_count;
                int byte_size = bufferView->byteLength;

                //vertices
                glGenBuffers(1,&vbo);
                glBindBuffer(GL_ARRAY_BUFFER,vbo);
                glBufferData(GL_ARRAY_BUFFER,byte_size,buffer->data.data()+offset,GL_STATIC_DRAW);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);
            }

            if(normal_accessor){
                BufferView* bufferView = &cmodel->bufferViews[normal_accessor->bufferView];
                Buffer* buffer = &cmodel->buffers[bufferView->buffer];

                int offset = normal_accessor->byteOffset + bufferView->byteOffset;
                int data_count = normal_accessor->count;
                int byte_size = bufferView->byteLength;

                //vertices
                glGenBuffers(1,&vbo);
                glBindBuffer(GL_ARRAY_BUFFER,vbo);
                glBufferData(GL_ARRAY_BUFFER,byte_size,buffer->data.data()+offset,GL_STATIC_DRAW);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);
            }

            if(texcoord_accessor){
                BufferView* bufferView = &cmodel->bufferViews[texcoord_accessor->bufferView];
                Buffer* buffer = &cmodel->buffers[bufferView->buffer];

                int offset = texcoord_accessor->byteOffset + bufferView->byteOffset;
                int data_count = texcoord_accessor->count;
                int byte_size = bufferView->byteLength;

                //texcoord
                glGenBuffers(1,&vbo);
                glBindBuffer(GL_ARRAY_BUFFER,vbo);
                glBufferData(GL_ARRAY_BUFFER,byte_size,buffer->data.data()+offset,GL_STATIC_DRAW);
                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,2*sizeof(float),nullptr);
            }

            if(primitive.indices>=0){
                indices_accessor = &cmodel->accessors[primitive.indices];
            }
            if(indices_accessor){
                BufferView* bufferView = &cmodel->bufferViews[indices_accessor->bufferView];
                Buffer* buffer = &cmodel->buffers[bufferView->buffer];

                int offset = indices_accessor->byteOffset + bufferView->byteOffset;
                int data_count = indices_accessor->count;
                int byte_size = bufferView->byteLength;

                mesh.indices_count = data_count;
                mesh.indices_type = indices_accessor->componentType;

                uint32_t ebo = 0 ;
                glGenBuffers(1,&ebo);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER,byte_size,buffer->data.data()+offset,GL_STATIC_DRAW);
            }

            if(joint_accessor){
                BufferView* bufferView = &cmodel->bufferViews[joint_accessor->bufferView];
                Buffer* buffer = &cmodel->buffers[bufferView->buffer];

                size_t offset = joint_accessor->byteOffset + bufferView->byteOffset;
                size_t data_count = joint_accessor->count;
                std::cout << "joints : " << data_count << std::endl;
                size_t byte_size = sizeof(uvec4)*data_count;

                std::vector<uint32_t> data;

                if(joint_accessor->componentType==GL_UNSIGNED_SHORT){
                    auto * ptr = (uint16_t*)(buffer->data.data()+offset);
                    int strip = bufferView->byteStride/2;
                    for (int i = 0; i < data_count; ++i) {
                        uvec4 indices{*(ptr+i*strip),*(ptr+i*strip+1),*(ptr+i*strip+2),*(ptr+i*strip+3)};
//                    std::cout << "j " << i << ":" << to_string(indices)  << std::endl;
                        for (int j = 0; j < 4; ++j) {
                            data.push_back(*(ptr+i*strip+j));
                        }
                    }
                }

                if(joint_accessor->componentType==GL_UNSIGNED_BYTE){
                    auto * ptr = (uint8_t*)(buffer->data.data()+offset);
                    for (int i = 0; i < data_count; ++i) {
                        uvec4 indices{*(ptr+i*4),*(ptr+i*4+1),*(ptr+i*4+2),*(ptr+i*4+3)};
//                    std::cout << "j " << i << ":" << to_string(indices) << std::endl;
                        for (int j = 0; j < 4; ++j) {
                            data.push_back(*(ptr+i*4+j));
                        }
                    }
                }

                //joints
                glGenBuffers(1,&vbo);
                glBindBuffer(GL_ARRAY_BUFFER,vbo);
                glBufferData(GL_ARRAY_BUFFER,byte_size,data.data(),GL_STATIC_DRAW);
                glEnableVertexAttribArray(3);
                glVertexAttribIPointer(3,4,GL_UNSIGNED_INT,sizeof(uvec4),nullptr);
            }

            if(weight_accessor){
                BufferView* bufferView = &cmodel->bufferViews[weight_accessor->bufferView];
                Buffer* buffer = &cmodel->buffers[bufferView->buffer];

                size_t offset = weight_accessor->byteOffset + bufferView->byteOffset;
                size_t data_count = weight_accessor->count;
                std::cout << "weights : " << data_count << std::endl;
                size_t byte_size = sizeof(vec4)*data_count;

                auto * ptr = (float*)(buffer->data.data()+offset);
                std::vector<float> data;
                for (int i = 0; i < data_count; ++i) {
                    vec4 weights{*(ptr+i*4),*(ptr+i*4+1),*(ptr+i*4+2),*(ptr+i*4+3)};
//                std::cout << "w" << i << ":" << to_string(weights) << std::endl;
                    for (int j = 0; j < 4; ++j) {
                        data.push_back(*(ptr+i*4+j));
                    }
                }

                //weights
                glGenBuffers(1,&vbo);
                glBindBuffer(GL_ARRAY_BUFFER,vbo);
                glBufferData(GL_ARRAY_BUFFER,byte_size,data.data(),GL_STATIC_DRAW);
                glEnableVertexAttribArray(4);
                glVertexAttribPointer(4,4,GL_FLOAT,GL_FALSE,sizeof(vec4),nullptr);
            }

            glBindVertexArray(0);

            if(primitive.material>-1){
                Material* cmat = &cmodel->materials[primitive.material];
                TextureInfo baseColorTextureInfo = cmat->pbrMetallicRoughness.baseColorTexture;
                if(baseColorTextureInfo.index>-1){
                    mesh.material.has_albedo_texture = 1;
                    Texture* baseColorTexture = &cmodel->textures[baseColorTextureInfo.index];
                    Sampler* sampler = &cmodel->samplers[baseColorTexture->sampler];
                    Image* image = &cmodel->images[baseColorTexture->source];

                    std::string& img_type = image->mimeType;

                    ivec3 shape{image->width,image->height,image->component};
                    int min_filter = sampler->minFilter;
                    int mag_filter = sampler->magFilter;
                    int warp_s = sampler->wrapS;
                    int warp_t = sampler->wrapT;

                    mesh.material.albedo_texture = Assets::LoadTexture(shape,{min_filter,mag_filter},{warp_s,warp_t},image->image.data());
                }

                TextureInfo metallicTextureInfo = cmat->pbrMetallicRoughness.metallicRoughnessTexture;
                if(metallicTextureInfo.index>-1){
                    mesh.material.has_metallic_roughness_texture = 1;
                    Texture* metallicTexture = &cmodel->textures[metallicTextureInfo.index];
                    Sampler* sampler = &cmodel->samplers[metallicTexture->sampler];
                    Image* image = &cmodel->images[metallicTexture->source];

                    std::string& img_type = image->mimeType;

                    ivec3 shape{image->width,image->height,image->component};
                    int min_filter = sampler->minFilter;
                    int mag_filter = sampler->magFilter;
                    int warp_s = sampler->wrapS;
                    int warp_t = sampler->wrapT;

                    mesh.material.metallic_roughness_texture = Assets::LoadTexture(shape,{min_filter,mag_filter},{warp_s,warp_t},image->image.data());
                }

                mesh.material.albedo_factor = make_vec4(cmat->pbrMetallicRoughness.baseColorFactor.data());
                mesh.material.metallic_factor = cmat->pbrMetallicRoughness.metallicFactor;
                mesh.material.roughness_factor = cmat->pbrMetallicRoughness.roughnessFactor;
            }

            model->primitives.push_back(mesh);
        }

        return model;
    }

    Skin* get_skin_component(level* scene,tinygltf::Model* cmodel,::entity_id entity,int skin_idx,std::unordered_map<int,::entity_id>& nodes){
        using namespace tinygltf;
        tinygltf::Skin& cskin = cmodel->skins[skin_idx];

        std::cout << "Skin : " << cskin.name << std::endl;

        auto skeleton= level_add_component(scene,entity,Skin{});
        skeleton->name = cskin.name;

        for (int i = 0; i < cskin.joints.size(); ++i) {
            skeleton->inverse_bind_matrices[i] = mat4(1.0f);
        }

        skeleton->joints_count = 0;
        for (auto& cjoint_id:cskin.joints) {
            auto joint_entity = nodes[cjoint_id];

            auto joint_spatial = level_get_component<Spatial3d>(scene,joint_entity);
            auto joint = level_add_component(scene,joint_entity,Joint{});

            if(joint_spatial->name.empty()){
                joint_spatial->name = "jointNode:"+std::to_string(cjoint_id);
            }

            if(joint == nullptr){
                WX_CORE_CRITICAL("load skeleton error");
                exit(-1001);
            }

            skeleton->joints[skeleton->joints_count++] = joint_entity;
        }

        Accessor * accessor = &cmodel->accessors[cskin.inverseBindMatrices];
        BufferView* bufferView = &cmodel->bufferViews[accessor->bufferView];
        Buffer* buffer = &cmodel->buffers[bufferView->buffer];

        size_t offset = accessor->byteOffset+bufferView->byteOffset;
        size_t data_count = accessor->count;
        auto* ptr = (float*)(buffer->data.data()+offset);
        for (int i = 0; i < data_count; ++i) {
            mat4 mat = make_mat4(ptr+i*16);
//        std::cout << "mat4 " << i << ":" << to_string(mat) << std::endl;
            skeleton->inverse_bind_matrices[i] = mat;
        }
        return skeleton;
    }

    ::entity_id get_node(level* scene,tinygltf::Model* cmodel,::entity_id parent,int node_idx,std::unordered_map<int,::entity_id>& nodes){
        using namespace tinygltf;

        tinygltf::Node& cnode = cmodel->nodes[node_idx];
        ::entity_id node_entity = nodes[node_idx];

        auto spatial = level_get_component<Spatial3d>(scene,node_entity);
        spatial->parent = parent;
        auto transform = level_get_component<Transform>(scene,node_entity);

        if(cnode.mesh!=-1){
            std::cout << "node mesh : " << cnode.mesh << std::endl;
            get_model_component(scene,cmodel,node_entity,cnode.mesh);
            level_add_component(scene,node_entity,ReceiveShadow{});
        }

        if(cnode.skin!=-1){
            std::cout << "node skin : " << cnode.skin << std::endl;
            get_skin_component(scene,cmodel,node_entity,cnode.skin,nodes);
        }

        for (auto child_idx:cnode.children) {
            auto child = get_node(scene,cmodel,node_entity,child_idx,nodes);
            spatial->children.push_back(child);
        }
        return node_entity;
    }

    ::entity_id create_node(level* scene,tinygltf::Node& cnode){
        using namespace tinygltf;
        ::entity_id node_entity = create_entity(scene);

        auto spatial = level_add_component(scene,node_entity,Spatial3d{});
        spatial->name = cnode.name;
        std::cout << "node : " << spatial->name << std::endl;

        auto transform = level_add_component(scene,node_entity,Transform{});

        if(cnode.translation.size() == 3){
            transform->position = make_vec3(cnode.translation.data());
        }
        if(cnode.rotation.size() == 4){
            transform->rotation = make_quat(cnode.rotation.data());
        }
        if(cnode.scale.size() == 3){
            transform->scale = make_vec3(cnode.scale.data());
        }
        return node_entity;
    }

    ::entity_id Assets::LoadAnimateModel(level* scene,const char *filename
                                         ,const char *name
                                         ,const Transform& transform) {
        using namespace tinygltf;

        Model cmodel;
        TinyGLTF loader;
        std::string err;
        std::string warn;

        bool ret = false;
        int len = 0;
        const FileInfo& fileInfo = AssetsLoader::FileInfo(filename);
        auto bytes = AssetsLoader::LoadRawData(filename, &len);
        if(fileInfo.ext=="glb"){
            ret = loader.LoadBinaryFromMemory(&cmodel,&err,&warn,bytes,fileInfo.size,"data");
        } else if(fileInfo.ext=="gltf"){
            ret = loader.LoadASCIIFromString(&cmodel, &err, &warn, reinterpret_cast<const char *>(bytes), len, "data");
        }

        if (!warn.empty()) {
            WX_CORE_WARN("tinyglft Warn: {}",warn);
        }

        if (!err.empty()) {
            WX_CORE_ERROR("tinyglft Err: {}",err);
        }

        if (!ret) {
            WX_CORE_ERROR("Failed to parse glTF");
            exit(-1002);
        }

        std::unordered_map<int,::entity_id> nodes;

        Scene* cscene = &cmodel.scenes[cmodel.defaultScene];

        ::entity_id root = 0;

        int cnode_idx = 0;
        for (auto& cnode:cmodel.nodes) {
            ::entity_id node = create_node(scene,cnode);
            nodes.insert(std::pair(cnode_idx++,node));
        }

        if(cscene->nodes.size()>1){
            root = create_entity(scene);
            auto root_spatial = level_add_component(scene,root,Spatial3d{});
            root_spatial->name = name;
            level_add_component(scene,root,transform);

            for (auto idx : cscene->nodes) {
                auto child = get_node(scene,&cmodel, root,idx,nodes);
                root_spatial->children.push_back(child);
            }
        }else if(cscene->nodes.size()==1){
            root = get_node(scene,&cmodel, 0,cscene->nodes[0],nodes);
            if(strcmp(name,"")!=0){
                auto root_spatial = level_get_component<Spatial3d>(scene,root);
                root_spatial->name = name;
            }
            auto root_transform = level_get_component<Transform>(scene,root);
            root_transform->position = transform.position;
            root_transform->rotation = transform.rotation;
            root_transform->scale = transform.scale;
        }else{
            WX_CORE_ERROR("Can't find any object!");
            return 0;
        }

        if(!cmodel.animations.empty()){
            auto root_spatial = level_get_component<Spatial3d>(scene,root);
            auto root_animator = level_add_component(scene,root,Animator{});

            for (auto& canimation:cmodel.animations) {
                std::cout << "Animation : " << canimation.name << std::endl;

                animation_t anim;
                memset(anim.name,'\0',sizeof(anim.name));
                if(canimation.name.empty()){
                    strcpy(anim.name,"default");
                }else{
                    strcpy(anim.name,canimation.name.data());
                }
                anim.channel_count = 0;
                for (auto& cchannel:canimation.channels) {
                    auto& channel = anim.channels[anim.channel_count++];
                    AnimationSampler* sampler = &canimation.samplers[cchannel.sampler];
                    channel.interpolation = 0;
                    if(sampler->interpolation=="STEP"){
                        channel.interpolation = 1;
                    }
                    if(sampler->interpolation=="CUBIC"){
                        channel.interpolation = 2;
                    }

                    channel.target = nodes[cchannel.target_node];

                    Accessor* input = &cmodel.accessors[sampler->input];
                    BufferView* inputBufferView = &cmodel.bufferViews[input->bufferView];
                    Buffer* inputBuffer = &cmodel.buffers[inputBufferView->buffer];

                    Accessor* output = &cmodel.accessors[sampler->output];
                    BufferView* outputBufferView = &cmodel.bufferViews[output->bufferView];
                    Buffer* outputBuffer = &cmodel.buffers[outputBufferView->buffer];

                    channel.keyframe_count = input->count;
                    for (int k = 0; k < input->count; ++k) {
                        auto& keyframe = channel.keyframe[k];

                        keyframe.time = *((float*)(inputBuffer->data.data() + input->byteOffset + inputBufferView->byteOffset) + k);
                        keyframe.translation = vec3(0.0f);
                        keyframe.rotation = quat(1.0f,0.0f,0.0f,0.0f);
                        keyframe.scale = vec3(1.0f);

                        if(cchannel.target_path=="translation"){
                            channel.has_translation = true;
                            keyframe.translation = make_vec3((float*)(outputBuffer->data.data()+output->byteOffset+outputBufferView->byteOffset) + k*3);
                        }
                        if(cchannel.target_path=="rotation"){
                            channel.has_rotation = true;
                            keyframe.rotation = make_quat((float*)(outputBuffer->data.data()+output->byteOffset+outputBufferView->byteOffset) + k*4);
                        }
                        if(cchannel.target_path=="scale"){
                            channel.has_scale = true;
                            keyframe.scale = make_vec3((float*)(outputBuffer->data.data()+output->byteOffset+outputBufferView->byteOffset) + k*3);
                        }
                    }
                }

                root_animator->animations.push_back(anim);
            }
        }
        return root;
    }

    uint32_t Assets::LoadTexture(ivec3 shape,ivec2 filter,ivec2 warp,const unsigned char *buffer) {
        int width = shape.x,height=shape.y,comp=shape.z;

        uint32_t texture;

        char buff[100]={0};
        sprintf(buff,"Image Info width=%d, height=%d, channels=%d\0",width,height,comp);
        std::cout << buff << std::endl;

        glGenTextures(1,&texture);
        //生成纹理
        glBindTexture(GL_TEXTURE_2D,texture);
        //为当前绑定的纹理对象设置环绕、过滤方式
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, warp.x>-1?warp.x:GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, warp.y>-1?warp.y:GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter.x>-1?filter.x:GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter.y>-1?filter.y:GL_LINEAR);
        if(comp==4){
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,buffer);
        }else if(comp==3){
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,buffer);
        }else{
            std::cerr << "Image Format Unsupported" << std::endl;
        }
        glGenerateMipmap(GL_TEXTURE_2D);
        return texture;
    }
}
