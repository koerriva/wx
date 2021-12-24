//
// Created by koerriva on 2021/12/21.
//

#ifndef WX_RENDERER_H
#define WX_RENDERER_H

#include <vector>
#include <tuple>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "window.h"
#include "assets.h"
#include "glad/glad.h"

namespace wx {
    using namespace std;
    using namespace glm;

    enum TerrainFaceType{
        SNOW=0,ROCK,DIRT,GRASS,SAND,SHORE,SHALLOW_WATER,DEEP_WATER
    };

    class Camera{
    private:
        vec3 position{0.f,0.f,0.f};
        vec3 front{0.f,0.f,-1.f};
        vec3 up{0.f,1.f,0.f};
        vec3 right{1.f,0.f,0.f};
        float pitch=0;
        float yaw = -90;
    public:
        explicit Camera(vec3 pos){
            this->position = pos;
        };
        ~Camera(){
            cout << "Drop Camera" << endl;
        }
        [[nodiscard]] mat4 GetViewMatrix() const{
            return lookAt(position,position+front,up);
        }

        void MoveForward(float factor){
            position += factor*front;
        }

        void MoveRight(float factor){
            position += factor*normalize(cross(front,up));
        }

        void Rotate(float xoffset,float yoffset){
            yaw -= xoffset;
            pitch -= yoffset;
            pitch = glm::clamp(pitch,-89.0f,89.f);
            glm::vec3 _front;
            _front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
            _front.y = sin(glm::radians(pitch));
            _front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
            this->front = glm::normalize(_front);
        }

        vec3 Position() const {
            return this->position;
        }

        vec3 Rotation(){
            return vec3{yaw,pitch,0};
        }
    };

    typedef struct {
        vec3 position{0};
        vec3 rotation{0};
        vec3 scale{1};
    } transform_t;

    typedef struct{
        uint32_t program_id;
        vec4 albedo{1.f};
        int has_albedo_texture = 0;
        uint32_t albedo_texture;
        int albedo_texture_index = 0;
        float metallic = 1.0;
        float roughness = 1.0;
        float ao = 1.0;
        int has_metallic_roughness_texture = 0;
        uint32_t metallic_roughness_texture;
        int metallic_roughness_texture_index;
    } material_t;

    typedef struct{
        uint32_t vao;
        int vertices_count = 0;
        int indices_count = 0;
        uint32_t indices_type;
        material_t materials[64];
        uint32_t material_count = 0;
    } mesh_t;

    typedef struct {
        mesh_t meshes[64];
        uint32_t mesh_count = 0;
        transform_t  transform;
    } model_t;

    typedef struct {
        float constant;
        float linear;
        float exponent;
    } attenuation_t;

    typedef enum {
        point=0,spot,directional,ambient
    } light_type_t;

    typedef struct {
        uint32_t fbo;
        uint32_t texture;
        uint32_t width;
        uint32_t height;
        uint32_t shader;
    } shadow_map_t;

    typedef struct {
        light_type_t type;
        vec3 color;
        vec3 position;
        vec3 direction;
        float intensity;
        float cutoff;
        attenuation_t attenuation;
        int has_shadow_map;
        shadow_map_t shadow_map;
        mat4 pv;
    } light_t;

    class Mesh
    {
    private:
        vector<float> vertices;//顶点顺序，逆时针为前，顺时针为后
        vector<unsigned int> indices;
        vector<float> normals;
        vector<float> texCoords;
        vector<float> colors;

        unsigned int vao=0;
        vector<unsigned int> vbos;
        unsigned int ebo=0;

    public:
        Mesh(vector<float>& vertices,vector<unsigned>& indices,vector<float> &normals,vector<float> &texCoords,vector<float>& colors);
        ~Mesh();

        void Draw() const;
        void Cleanup() const;

        static Mesh Sphere(float r,int sectors,int stacks);
        static void DumpPNGFile(int width,int height,vector<float>& colors);
    };

    class Texture{
    private:
        const unsigned char* buffer = nullptr;
        int len,width,height,comp;
        unsigned int texture;
    public:
        Texture(const unsigned char* buffer,int len);
        ~Texture(){
            cout << "Drop Texture" << endl;
        }

        static uint32_t Load(const unsigned char* buffer,int len,ivec2 filter,ivec2 warp);
        static shadow_map_t LoadDepthMap(uint32_t width,uint32_t height);
        void Bind() const;
        void Cleanup();
    };

    enum ShaderType{
        VERTEX_SHADER=1,GEOMETRY_SHADER,FRAGMENT_SHADER
    };

    class ShaderProgram{
    private:
        uint32_t value;
        static unordered_map<string,int> uniforms;
        static unsigned int CreateShader(GLuint type,const char* source);

    public:
        operator uint32_t() {return value;}
        ShaderProgram operator=(uint32_t v){
            ShaderProgram s;
            s.value = v;
            return s;
        }

        static uint32_t LoadShader(const char* name);

        static void Bind(uint32_t pid) ;
        static void Unbind() ;
        static void Cleanup(uint32_t pid);

        static void SetFloat(uint32_t pid,const string& name,float value);
        static void SetMat4(uint32_t pid,const string& name,float* value);
        static void SetVec4(uint32_t pid,const string& name,float* value);
        static void SetVec3(uint32_t pid,const string& name,float* value);
        static void SetInt(uint32_t pid, const string& name, int value);

        static void SetAttenuation(uint32_t pid, const string& _name, attenuation_t value);
        static void SetLight(uint32_t pid, const string& _name, vector <light_t> &lights);
    };

    struct Patch{
        bool loaded = false;
        float points[12]{};
        float vertex_count = 4;
        unsigned int indices[6] = {0,3,2,0,2,1};
        unsigned int vao{},vbo{},ebo{};

        void Upload();
        void Draw();
    };
    struct QuadTreeNode{
        unsigned long id=0;
        //N,E,S,W
        QuadTreeNode* neighbors[4]{};
        size_t neighbors_count=0;
        QuadTreeNode* children[4]{};
        size_t children_count=0;
        QuadTreeNode* parent{};
        //深度
        size_t depth{};
        //渲染元
        Patch patch;
        //中心点
        vec3 center{};
        //边长
        float bound{};
    };

    class Terrain{
    private:
        unsigned long MAX_CHUNK_COUNT = 65535;
        unsigned long next_id = 0;
        QuadTreeNode* chunks = static_cast<QuadTreeNode *>(malloc(sizeof(QuadTreeNode) * MAX_CHUNK_COUNT));

        QuadTreeNode* CreateNewChunk(size_t depth, vec3 center, float bound, QuadTreeNode* parent);
        void SplitChunk(QuadTreeNode* parent);
        void SplitChunk(QuadTreeNode* parent,int depth);
        QuadTreeNode* target_chunk;
        int target_depth=1;
    public:
        Terrain();

        virtual ~Terrain() {
            cout << "Drop Terrain" << endl;
        }

        void Init(){
        }

        void Update(int depth){
            target_depth = depth;
            SplitChunk(target_chunk,target_depth);
//            const size_t size = next_id;
//            for (size_t i = 0; i < size; ++i) {
//                auto& chunk = chunks[i];
//                if(chunk.depth<target_depth&&chunk.children_count==0){
//                    SplitChunk(&chunk);
//                }
//            }
        }

        void Draw(){
            const size_t size = next_id;
            for (size_t i = 0; i < size; ++i) {
                auto& chunk = chunks[i];
                if(chunk.depth<=target_depth){
                    chunk.patch.Draw();
                }
            }
        }

        void Cleanup(){
            delete chunks;
        }

        [[nodiscard]] size_t GetChunkSize() const {
            return next_id;
        }
    };

    class Renderer
    {
    private:
        bool WIREFRAME_MODE = false;
        bool SHADER_MODE = true;

        float frame_time = 0;
        uint32_t frame_count = 0;
    public:
        Renderer(/* args */);
        ~Renderer();

        void Init();
        void SetWireframeMode();
        void SetShaderMode();
        void Render(const Window* window,const Camera* camera,const vector<Mesh>& meshList,const vector<Texture>& textures,ShaderProgram shaderProgram);
        void Render(const Window* window,const Camera* camera,Terrain* terrain,ShaderProgram shaderProgram);
        void Render(const Window* window,vector<model_t>& models,vector<light_t>& lights,float delta);
        void Render(const Window* window,const Camera* camera,vector<model_t>& models,vector<light_t>& lights,float delta);

        static vector<model_t> LoadModelFromGLTF(const char* filename);
    };

    class Debug{
    private:
        uint32_t shaderProgram;
        vector<float> vertices;
        GLuint VAO{},VBO{};
    public:
        Debug();
        void PrintScreen(vec2 pos,const char* text,vec3 color);
    };
}

#endif //WX_RENDERER_H
