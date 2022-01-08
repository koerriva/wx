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
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

namespace wx {
    Mesh trav_get_model(scene_t* scene,tinygltf::Model* cmodel,object_t* node,int mesh_idx,model_t* model,std::unordered_map<int,object_t*>& nodes);
    skeleton_t* trav_get_skin(scene_t* scene,tinygltf::Model* cmodel,object_t* node,int skin_idx,skeleton_t* skeleton,std::unordered_map<int,object_t*>& nodes);
    ::entity_id trav_get_node(scene_t* scene,tinygltf::Model* cmodel,object_t* parent,int node_idx,std::unordered_map<int,object_t*>& nodes);

    Mesh trav_get_model(scene_t* scene,tinygltf::Model* cmodel,object_t* node,int mesh_idx,model_t* model,std::unordered_map<int,object_t*>& nodes){
        using namespace tinygltf;
        //load mesh
        Mesh* cmesh = &cmodel->meshes[mesh_idx];
        std::cout << "Mesh : " << cmesh->name << std::endl;

        model->mesh_count = 0;
        for (auto& primitive:cmesh->primitives) {
            mesh_t& mesh = model->meshes[model->mesh_count++];
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

            mesh.material.baseColor = vec4(1.0f);
            if(primitive.material>-1){
                Material* cmat = &cmodel->materials[primitive.material];
                TextureInfo baseColorTextureInfo = cmat->pbrMetallicRoughness.baseColorTexture;
                if(baseColorTextureInfo.index>-1){
                    mesh.material.hasBaseColorTexture = 1;
                    Texture* baseColorTexture = &cmodel->textures[baseColorTextureInfo.index];
                    Sampler* sampler = &cmodel->samplers[baseColorTexture->sampler];
                    Image* image = &cmodel->images[baseColorTexture->source];

                    std::string& img_type = image->mimeType;

                    ivec3 shape{image->width,image->height,image->component};
                    int min_filter = sampler->minFilter;
                    int mag_filter = sampler->magFilter;
                    int warp_s = sampler->wrapS;
                    int warp_t = sampler->wrapT;

                    mesh.material.baseColorTexture = Assets::LoadTexture(shape,{min_filter,mag_filter},{warp_s,warp_t},image->image.data());
                }

                mesh.material.baseColor = make_vec4(cmat->pbrMetallicRoughness.baseColorFactor.data());
            }
        }
        return model;
    }

    skeleton_t* trav_get_skin(scene_t* scene,tinygltf::Model* cmodel,object_t * parent,int skin_idx,skeleton_t* skeleton,std::unordered_map<int,object_t*>& nodes){
        using namespace tinygltf;
        Skin& cskin = cmodel->skins[skin_idx];

        std::cout << "Skin : " << cskin.name << std::endl;

        for (int i = 0; i < cskin.joints.size(); ++i) {
            skeleton->inverse_bind_matrices[i] = mat4(1.0f);
        }

        skeleton->joints_count = 0;
        for (auto& cjoint_id:cskin.joints) {
            object_t * node = nodes[cjoint_id];
            node->jointed = 1;

            if(strcmp(node->name,"")==0){
                memset(node->name,'\0',sizeof(node->name));
                strcpy(node->name,("jointNode:"+std::to_string(cjoint_id)).c_str());
            }

            if(node == nullptr){
                std::cerr << "load skeleton error" << std::endl;
                exit(-1001);
            }

            skeleton->joints[skeleton->joints_count++] = node;
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
            skeleton->joints[i]->jointed_matrices = mat;
        }
        return skeleton;
    }

    ::entity_id trav_get_node(scene_t* scene,tinygltf::Model* cmodel,object_t* parent,int node_idx,std::unordered_map<int,object_t*>& nodes){
        using namespace tinygltf;

        Node& cnode = cmodel->nodes[node_idx];
        object_t * node = nodes[node_idx];

        node->parent = parent;

        if(cnode.mesh!=-1){
            std::cout << "node mesh : " << cnode.mesh << std::endl;
            node->has_model = 1;
            node->model = new model_t;

            scene->models[scene->model_count++] = node;

            trav_get_model(scene,cmodel,node,cnode.mesh,node->model,nodes);
        }

        if(cnode.skin!=-1){
            std::cout << "node skin : " << cnode.skin << std::endl;
            node->has_skin = 1;
            node->skeleton = new skeleton_t;

            trav_get_skin(scene,cmodel,node,cnode.skin,node->skeleton,nodes);
        }

        for (auto child_idx:cnode.children) {
            auto child = trav_get_node(scene,cmodel,node,child_idx,nodes);
            node->children[node->children_count++] = child;
        }
        return node;
    }

    ::entity_id * create_node(scene_t* scene,tinygltf::Node& cnode){
        using namespace tinygltf;
        object_t * node = &scene->objects[scene->object_count++];
        node->id = scene->object_count;
        memset(node->name,'\0',sizeof(node->name));
        if(cnode.name.empty()){
            strcpy(node->name,("node_"+std::to_string(node->id)).c_str());
        }else{
            strcpy(node->name, cnode.name.c_str());
        }
        std::cout << "node : " << node->name << std::endl;

        if(cnode.translation.size() == 3){
            node->transform.position = make_vec3(cnode.translation.data());
        }
        if(cnode.rotation.size() == 4){
            node->transform.rotation = make_quat(cnode.rotation.data());
        }
        if(cnode.scale.size() == 3){
            node->transform.scale = make_vec3(cnode.scale.data());
        }
        return node;
    }

    ::entity_id Assets::LoadAnimateModel(const char *filename) {
        using namespace tinygltf;

        Model cmodel;
        TinyGLTF loader;
        std::string err;
        std::string warn;

        bool ret = loader.LoadASCIIFromFile(&cmodel, &err, &warn, filename);
//bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, argv[1]); // for binary glTF(.glb)

        if (!warn.empty()) {
            printf("Warn: %s\n", warn.c_str());
        }

        if (!err.empty()) {
            printf("Err: %s\n", err.c_str());
        }

        if (!ret) {
            printf("Failed to parse glTF\n");
            return nullptr;
        }

        std::unordered_map<int,object_t *> nodes;

        Scene* cscene = &cmodel.scenes[cmodel.defaultScene];

        object_t * root = nullptr;

        int cnode_idx = 0;
        for (auto& cnode:cmodel.nodes) {
            object_t * node = create_node(scene,cnode);
            nodes.insert(std::pair(cnode_idx++,node));
        }

        if(cscene->nodes.size()>1){
            root = &scene->objects[scene->object_count++];
            root->id = scene->object_count;
            memset(root->name,'\0',sizeof(root->name));
            strcpy(root->name,cscene->name.c_str());

            for (auto idx : cscene->nodes) {
                auto* child = trav_get_node(scene,&cmodel, root,idx,nodes);
                root->children[root->children_count++] = child;
            }
        }else if(cscene->nodes.size()==1){
            root = trav_get_node(scene,&cmodel, nullptr,cscene->nodes[0],nodes);
        }else{
            std::cerr << "Can't find any object!" << std::endl;
            return nullptr;
        }

        if(!cmodel.animations.empty()){
            root->has_animation = 1;
            for (auto& canimation:cmodel.animations) {
                std::cout << "Animation : " << canimation.name << std::endl;

                animation_t& anim = root->animations[root->animation_count++];
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
                    channel.origin = channel.target->transform;

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
            }
            root->animator = new Animator(root);
        }

        scene->roots[scene->root_count++] = root;
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
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
