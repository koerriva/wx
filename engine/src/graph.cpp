//
// Created by koerriva on 2021/12/27.
//
#include "log.h"
#include "renderer.h"
#include "font.h"

#include <stb/stb_image.h>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/normal.hpp>
#include <glm/gtc/noise.hpp>
#include "noise.h"
#include "SimplexNoise.h"

namespace wx {
    //MeshLoader
    uint32_t Assets::UnitQuad() {
        uint32_t vao,vbo;
        glGenVertexArrays(1,&vao);
        glBindVertexArray(vao);

        glGenBuffers(1,&vbo);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);

        float data[24] = {
                -1.f,  1.f, 0.0f,0.0f,//
                -1.f, -1.f, 0.0f,1.0f,
                1.f,  1.f, 1.0f,0.0f,
                1.f,  1.f, 1.0f,0.0f,
                -1.f, -1.f, 0.0f,1.0f,
                1.f, -1.f, 1.0f,1.0f,
        };

        glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(GLfloat), data, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,4*sizeof(float),nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);
        return vao;
    }

    Mesh::primitive_t Assets::GenMesh(SurfaceData &surfaceData) {
        uint32_t vao,vbo,ebo;
        glGenVertexArrays(1,&vao);
        glBindVertexArray(vao);

        glGenBuffers(1,&vbo);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER, surfaceData.vertices.size() * 3 * sizeof(GLfloat), surfaceData.vertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);

        glGenBuffers(1,&vbo);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER, surfaceData.normals.size() * 3 * sizeof(GLfloat), surfaceData.normals.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);

        glGenBuffers(1,&vbo);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER, surfaceData.texCoords.size() * 2 * sizeof(GLfloat), surfaceData.texCoords.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,2*sizeof(float),nullptr);

        glGenBuffers(1,&ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,surfaceData.indices.size()*sizeof(GLushort), surfaceData.indices.data(),GL_STATIC_DRAW);

        glBindVertexArray(0);

        Mesh::primitive_t primitive;
        primitive.vertices_count = surfaceData.vertices.size();
        primitive.indices_count = surfaceData.indices.size();
        primitive.vao = vao;
        primitive.material.metallic_factor = 0.1;
        primitive.material.roughness_factor = 0.9;

        return primitive;
    }

    uint32_t TextureLoader::Load(const char *name) {
        int len = 0;
        auto buffer = AssetsLoader::LoadTexture(name,&len);
        return Load(buffer,len,{GL_LINEAR,GL_LINEAR},{GL_REPEAT,GL_REPEAT});
    }

    uint32_t TextureLoader::Load(const unsigned char *buffer, int len, ivec2 filter, ivec2 warp) {
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

    shadow_map_t TextureLoader::LoadDepthMap(uint32_t width, uint32_t height) {
        GLuint depthMapFBO;
        glGenFramebuffers(1, &depthMapFBO);

        GLuint depthMap;
        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                     width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

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

    shadow_map_t TextureLoader::LoadVarianceDepthMap(uint32_t width, uint32_t height) {
        GLuint depthMapFBO;
        GLuint depthMapRBO;
        glGenFramebuffers(1, &depthMapFBO);
        glGenRenderbuffers(1, &depthMapRBO);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glBindRenderbuffer(GL_RENDERBUFFER, depthMapRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthMapRBO);
        glEnable(GL_DEPTH_TEST);

        GLuint depthMap;
        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RG, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depthMap, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        shadow_map_t shadowMap;
        shadowMap.fbo = depthMapFBO;
        shadowMap.texture = depthMap;
        shadowMap.width = width;
        shadowMap.height = height;
        return shadowMap;
    }

    shadow_map_t TextureLoader::LoadDepthCubeMap(uint32_t width, uint32_t height) {
        GLuint depthMapFBO;
        glGenFramebuffers(1, &depthMapFBO);

        GLuint depthMap;
        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthMap);

        for (GLuint i = 0; i < 6; ++i){
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                         width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        //只关心深度缓冲
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
            WX_CORE_ERROR("Framebuffer not complete!");
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        shadow_map_t shadowMap;
        shadowMap.fbo = depthMapFBO;
        shadowMap.texture = depthMap;
        shadowMap.width = width;
        shadowMap.height = height;
        return shadowMap;
    }

    cubemap_t TextureLoader::LoadRendererCubeMap(uint32_t width, uint32_t height) {
        GLuint cubeMapFBO;
        glGenFramebuffers(1, &cubeMapFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, cubeMapFBO);

        GLuint cubeMap;
        glGenTextures(1, &cubeMap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

        for (GLuint i = 0; i < 6; ++i){
//            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        for (GLuint i = 0; i < 6; ++i) {
            glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0 + i,GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,cubeMap,0);
        }

//        glFramebufferTexture(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,cubeMap,0);

        uint32_t rbo;
        glGenRenderbuffers(1,&rbo);
        glBindRenderbuffer(GL_RENDERBUFFER,rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
            WX_CORE_ERROR("Framebuffer not complete!");
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        cubemap_t cubemap;
        cubemap.fbo = cubeMapFBO;
        cubemap.texture = cubeMap;
        cubemap.width = width;
        cubemap.height = height;
        return cubemap;
    }

    unordered_map<string,int> ShaderProgram::uniforms;
    unsigned int ShaderProgram::CreateShader(GLuint type,const char* source) {
        unsigned int shader = glCreateShader(type);
        switch (type) {
            case GL_VERTEX_SHADER:
            case GL_FRAGMENT_SHADER:
            case GL_GEOMETRY_SHADER:
                glShaderSource(shader,1,&source, nullptr);
                break;
            default:
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

    unsigned int ShaderProgram::LoadShader(const char* name,bool geom) {
        unsigned int program = glCreateProgram();
        const char* vertShaderSource = AssetsLoader::LoadShader(name,ShaderType::VERTEX_SHADER);
        const char* fragShaderSource = AssetsLoader::LoadShader(name,ShaderType::FRAGMENT_SHADER);
        unsigned vertexShader = CreateShader(GL_VERTEX_SHADER,vertShaderSource);
        unsigned fragmentShader = CreateShader(GL_FRAGMENT_SHADER,fragShaderSource);
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        unsigned geomShader;
        if(geom){
            const char* geomShaderSource = AssetsLoader::LoadShader(name,ShaderType::GEOMETRY_SHADER);
            geomShader = CreateShader(GL_GEOMETRY_SHADER,geomShaderSource);
            glAttachShader(program, geomShader);
        }
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
        if(geom){
            glDeleteShader(geomShader);
        }
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

    int ShaderProgram::FindUniformLocation(uint32_t pid, const string &_name) {
        int location = 0;
        string name = to_string(pid) + "_" + _name;
        if(uniforms.count(name)==0){
//            cout << "Find Uniform : " << name << endl;
            location = glGetUniformLocation(pid,_name.c_str());
//            cout << "Uniform[" << name << "] Location=" << location << endl;
            uniforms[name]=location;
        }else{
            location = uniforms[name];
        }
        return location;
    }

    void ShaderProgram::SetFloat(uint32_t pid,const string& _name, float value) {
        glUniform1f(FindUniformLocation(pid,_name),value);
    }
    void ShaderProgram::SetDouble(uint32_t pid,const string& _name, double value) {
        glUniform1d(FindUniformLocation(pid,_name),value);
    }
    void ShaderProgram::SetMat4(uint32_t pid,const string& _name, float *value) {
        glUniformMatrix4fv(FindUniformLocation(pid,_name),1,GL_FALSE,value);
    }
    void ShaderProgram::SetMat3(uint32_t pid,const string& _name, float *value) {
        glUniformMatrix3fv(FindUniformLocation(pid,_name),1,GL_FALSE,value);
    }
    void ShaderProgram::SetVec4(uint32_t pid,const string& _name, float *value) {
        glUniform4fv(FindUniformLocation(pid,_name),1,value);
    }
    void ShaderProgram::SetVec3(uint32_t pid,const string& _name, float *value) {
        glUniform3fv(FindUniformLocation(pid,_name),1,value);
    }
    void ShaderProgram::SetVec2(uint32_t pid,const string& _name, float *value){
        glUniform2fv(FindUniformLocation(pid,_name),1,value);
    }
    void ShaderProgram::SetInt(uint32_t pid, const string& _name, int value) {
        glUniform1i(FindUniformLocation(pid,_name),value);
    }
    void ShaderProgram::SetAttenuation(uint32_t pid, const string& _name, Light::attenuation_t value){
        ShaderProgram::SetFloat(pid,_name+".constant",value.constant);
        ShaderProgram::SetFloat(pid,_name+".exponent",value.exponent);
        ShaderProgram::SetFloat(pid,_name+".linear",value.linear);
    }
    void ShaderProgram::SetLight(uint32_t pid, const string& _name, vector<Light*> &lights) {
        for (int i = 0; i < lights.size(); ++i) {
            auto& light = lights[i];
            string name = "lights["+ to_string(i)+"]";
            ShaderProgram::SetInt(pid,name+".type",light->type);
            ShaderProgram::SetInt(pid,name+".state",light->state);
            ShaderProgram::SetVec3(pid,name+".color", value_ptr(light->color));
            ShaderProgram::SetVec3(pid,name+".position", value_ptr(light->position));
            ShaderProgram::SetVec3(pid,name+".direction", value_ptr(light->direction));
            ShaderProgram::SetFloat(pid,name+".intensity",light->intensity);
            ShaderProgram::SetFloat(pid,name+".cutoff",light->cutoff);
            ShaderProgram::SetInt(pid,name+".has_shadow_map",light->has_shadow_map);
            ShaderProgram::SetFloat(pid,name+".near_plane",light->near_plane);
            ShaderProgram::SetFloat(pid,name+".far_plane",light->far_plane);
            ShaderProgram::SetInt(pid,name+".shadow_map_index",light->shadow_map_index);
            ShaderProgram::SetAttenuation(pid,name+".att",light->attenuation);
        }
    }

/**
 * Debug
 */
    struct UChar{
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

    void Debug::PrintScreen(vec2 pos, const wchar_t* text, vec3 color) {
        glEnable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        ShaderProgram::Bind(shaderProgram);
        ShaderProgram::SetVec3(shaderProgram,"color", value_ptr(color));
        mat4 P = ortho(0.f,1280.f,720.f,0.f);
        ShaderProgram::SetMat4(shaderProgram, "P", value_ptr(P));
        ShaderProgram::SetInt(shaderProgram,"color_texture",0);
        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);

        wstring chars = text;
        for (auto it=chars.begin();it!=chars.end();++it) {
            UChar c = Font::GetChar(*it);
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