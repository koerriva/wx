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

    Mesh::primitive_t Assets::UnitSubQuad(int subdivision,float base_radius,size_t octaves
                                          ,float frequency
                                          ,float amplitude
                                          ,float lacunarity
                                          ,float persistence) {

        SimplexNoise simplexNoise{frequency,amplitude,lacunarity,persistence};

        uint32_t vao,vbo;
        glGenVertexArrays(1,&vao);
        glBindVertexArray(vao);

        std::vector<vec3> vertices;
        std::vector<vec3> normals;
        std::vector<vec2> texcoords;

        int step = subdivision/2;
        float delta = 4.f/float(subdivision);
        float td = 2.f/ float(subdivision);

        const auto spherified  = [](vec3& p)->vec3 {
            float x=p.x,y=p.y,z=p.z;
            float xx = x*x;float yy = y*y;float zz = z*z;

            x = x*sqrt(1.f-yy/2.f-zz/2.f+yy*zz/3.f);
            y = y*sqrt(1.f-zz/2.f-xx/2.f+zz*xx/3.f);
            z = z*sqrt(1.f-xx/2.f-yy/2.f+xx*yy/3.f);
            return vec3{x,y,z};
        };

        //Z+ Z- , X+ X- , Y+ Y-
        vec3 start[6] = {{-1.f,1.f,1.f},{1.f,1.f,-1.f}
        ,{1.f,1.f,1.f},{-1.f,1.f,-1.f}
        ,{-1.f,1.f,-1.f},{1.f,-1.f,1.f}
        };
        vec3 right[6] = {{1.f,0.f,0.f},{-1.f,0.f,0.f}
        ,{0.f,0.f,-1.f},{0.f,0.f,1.f}
        ,{1.f,0.f,0.f},{-1.f,0.f,0.f}
        };
        vec3 up[6] = {{0.f,1.f,0.f},{0.f,1.f,0.f}
        ,{0.f,1.f,0.f},{0.f,1.f,0.f}
        ,{0.f,0.f,-1.f},{0.f,0.f,1.f}
        };

        for (int i = 0; i < 6; ++i) {
            for (int h = 0; h < step; ++h) {
                for (int v = 0; v < step; ++v) {
                    vec3 v10 = start[i] + float(v)*delta*right[i]-float(h)*delta*up[i];
                    vec2 t10{float(v)*td,float(h)*td};

                    vec3 v11 = start[i] + float(v)*delta*right[i]-float(h+1)*delta*up[i];
                    vec2 t11{float(v)*td,float(h+1)*td};

                    vec3 v12 = start[i] + float(v+1)*delta*right[i]-float(h+1)*delta*up[i];
                    vec2 t12{float(v+1)*td,float(h+1)*td};

                    vec3 p0[3]={spherified(v10),spherified(v11),spherified(v12)};
                    for (auto & p:p0) {
                        float r = simplexNoise.fractal(octaves,p.x,p.y,p.z);
                        r = (r+1.f)*0.5f;
                        if(r<base_radius){
                            r = base_radius;
                        }
                        p = p*r;
                        vertices.push_back(p);
                    }
                    vec2 t0[3]={t10,t11,t12};
                    for (auto & t:t0) {
                        texcoords.push_back(t);
                    }

                    vec3 v20 = start[i] + float(v)*delta*right[i]-float(h)*delta*up[i];;
                    vec2 t20{float(v)*td,float(h)*td};

                    vec3 v21 = start[i] + float(v+1)*delta*right[i]-float(h+1)*delta*up[i];;
                    vec2 t21{float(v+1)*td,float(h+1)*td};

                    vec3 v22 = start[i] + float(v+1)*delta*right[i]-float(h)*delta*up[i];;
                    vec2 t22{float(v+1)*td,float(h)*td};

                    vec3 p1[3]={spherified(v20),spherified(v21),spherified(v22)};
                    for (auto & p : p1) {
                        float r = simplexNoise.fractal(octaves,p.x,p.y,p.z);
                        r = (r+1.f)*0.5f;
                        if(r<base_radius){
                            r = base_radius;
                        }
                        p = p*r;
                        vertices.push_back(p);
                    }
                    vec2 t1[3]={t20,t21,t22};
                    for (auto & t : t1) {
                        texcoords.push_back(t);
                    }

                    vec3 f0_normal = triangleNormal(p0[0],p0[1],p0[2]);
                    vec3 f1_normal = triangleNormal(p1[0],p1[1],p1[2]);
                    vec3 normal = normalize(f0_normal + f1_normal);

                    normals.push_back(normal);
                    normals.push_back(normal);
                    normals.push_back(normal);
                    normals.push_back(normal);
                    normals.push_back(normal);
                    normals.push_back(normal);
                }
            }
        }

        glGenBuffers(1,&vbo);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * 3 * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1,&vbo);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * 3 * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1,&vbo);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER, texcoords.size() * 2 * sizeof(GLfloat), texcoords.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,2*sizeof(float),nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);

        Mesh::primitive_t primitive;
        primitive.vertices_count = vertices.size();
        primitive.vao = vao;
        primitive.material.metallic_factor = 0.01;
        primitive.material.roughness_factor = 0.99;
        return primitive;
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