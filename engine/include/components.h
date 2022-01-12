//
// Created by koerriva on 2022/1/8.
//

#ifndef WX_COMPONENTS_H
#define WX_COMPONENTS_H

#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

typedef uint32_t entity_id;

typedef uint32_t VAO;
typedef uint32_t TEXTURE;

namespace wx {
    using namespace glm;

    struct MainCamera{};
    struct Camera {
        vec3 position{0.f, 0.f, 0.f};
        vec3 front{0.f, 0.f, -1.f};
        vec3 up{0.f, 1.f, 0.f};
        vec3 right{1.f, 0.f, 0.f};
        float pitch = 0;
        float yaw = -90;

        [[nodiscard]] quat GetRotation() const{
            return quat{vec3(radians(yaw), radians(pitch), radians(0.f))};
        }
    };

    struct Transform {
        Transform* parent = nullptr;
        int has_parent = 0;
        vec3 position{0.0f};
        quat rotation{1.0f,0.0f,0.0f,0.0f};
        vec3 scale{1.0f};

        [[nodiscard]] mat4 GetLocalTransform() const {
            mat4 T = translate(mat4{1.0f},position);
            mat4 R = mat4_cast(rotation);
            mat4 S = glm::scale(mat4{1.0f},scale);
            return T*R*S;
        }

        [[nodiscard]] mat4 GetGlobalTransform() const {
            mat4 local = GetLocalTransform();
            if(has_parent){
                return parent->GetGlobalTransform()*local;
            }else{
                return local;
            }
        }
    };

    struct AnimatedTransform {
        Transform* parent = nullptr;
        int has_parent = 0;
        vec3 position{0.0f};
        quat rotation{1.0f,0.0f,0.0f,0.0f};
        vec3 scale{1.0f};

        [[nodiscard]] mat4 GetLocalTransform() const {
            mat4 T = translate(mat4{1.0f},position);
            mat4 R = mat4_cast(rotation);
            mat4 S = glm::scale(mat4{1.0f},scale);
            return T*R*S;
        }

        [[nodiscard]] mat4 GetGlobalTransform() const {
            mat4 local = GetLocalTransform();
            if(has_parent){
                return parent->GetGlobalTransform()*local;
            }else{
                return local;
            }
        }
    };

    typedef struct material_t {
        vec4 albedo_factor{1.f};
        int has_albedo_texture = 0;
        uint32_t albedo_texture;
        int albedo_texture_index = 0;
        float metallic_factor = 0.001;
        float roughness_factor = 0.001;
        float ao = 1.0;
        int has_metallic_roughness_texture = 0;
        uint32_t metallic_roughness_texture;
        int metallic_roughness_texture_index = 1;
    } material_t;

    typedef struct shadow_map_t{
        uint32_t fbo;
        uint32_t texture;
        uint32_t width;
        uint32_t height;
    } shadow_map_t;

    struct CastShadow{};
    struct ReceiveShadow{};

    struct Light {
        enum Type{
            point = 0, spot, directional, ambient
        };

        typedef struct {
            float constant;
            float linear;
            float exponent;
        } attenuation_t;

        Type type;
        int state;//0-关闭,1-开启
        vec3 color;
        vec3 position;
        vec3 direction;
        float intensity;
        float cutoff;
        attenuation_t attenuation;
        int has_shadow_map = 0;
        shadow_map_t shadow_map;
        int shadow_map_index = 0;
        mat4 p{1.f};
        mat4 v{1.f};
        float near_plane;
        float far_plane;
    };

    struct Joint{};
#define MAX_JOINT_COUNT 64
    struct Skin {
        std::string name;
        int joints_count = 0;
        entity_id joints[MAX_JOINT_COUNT]{};
        mat4 inverse_bind_matrices[MAX_JOINT_COUNT]{};
    };

    struct Mesh{
        std::string name;
        struct primitive_t {
            VAO vao{};
            int vertices_count = 0;
            int indices_count = 0;
            uint32_t indices_type = 0x1403;
            material_t material;
        };
        std::vector<primitive_t> primitives;
    };

    typedef struct keyframe_t {
        float time = 0;
        vec3 translation{0};
        quat rotation{1,0,0,0};
        vec3 scale{1};
    } keyframe_t;

#define MAX_KEYFRAME_COUNT 120

    typedef struct channel_t {
        int keyframe_count = 0;
        keyframe_t keyframe[120]{};
        int interpolation = 0;//0-liber,1-step,2-cubic
        bool has_translation = false;
        bool has_rotation = false;
        bool has_scale = false;
        //must have AnimatedTransform component
        ::entity_id target = 0;
    } channel_t;

    typedef struct animation_t {
        int channel_count = 0;
        channel_t channels[40]{};
        char name[45]{0};
    } animation_t;

    struct Canvas {
        VAO vao;
        TEXTURE texture;
        vec2 position{0};
        vec2 size{0};
    };

    struct Spatial3d{
        std::string name;
        ::entity_id parent;
        std::vector<entity_id> children;
        std::vector<animation_t> animations;
    };

    //resource
    struct VPMatrices {
        mat4 view{1.0f};
        mat4 project{1.0f};
        mat4 ortho{1.0f};
    };
    struct FrameState{
        float delta_time=0.0f;
        float total_time=0.0f;
        int total_count=0.0f;
    };

    struct PBRShader{
        uint32_t id;
    };

    struct FlatShader{
        uint32_t id;
    };

    struct FontShader{
        uint32_t id;
    };

    struct DepthShader{
        uint32_t id;
    };

    struct DepthCubeShader{
        uint32_t id;
    };

    struct WindowConfig {
        std::string title;
        int width=1280;
        int height=720;
        bool vSync = true;
    };

    struct InputState {
        const float cursor_sensitivity = 0.05;
        enum KeyCode {
            ESC = 256,ENTER,TAB,
            RIGHT=262,LEFT,DOWN,UP,
            F1 = 290,
            F2,F3,F4,F5,F6,F7,F8,F9,F10,F11,F12,
            Num0 = 48,Num1,Num2,Num3,Num4,Num5,Num6,Num7,Num8,Num9,
            A=65,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z
        };

        enum MouseCode {
            M_LEFT = 0,
            M_RIGHT = 1,M_MID=2
        };

        enum KeyState{
            RELEASE=0,PRESS,REPEAT,//GLFW_PRESS=1,GLFW_RELEASE=0
        };

        int last_key_state[349] = {0};
        int curr_key_state[349] = {0};

        int last_button_state[3] = {0};
        int curr_button_state[3] = {0};

        vec2 last_cursor_pos{0};
        vec2 curr_cursor_pos{0};

        bool GetKeyPressed(int key){
            return last_key_state[key]==PRESS&&curr_key_state[key]==RELEASE;
        }

        bool GetKeyDown(int key){
            return curr_key_state[key]==PRESS;
        }

        bool GetKeyUp(int key){
            return curr_key_state[key]==RELEASE;
        }

        bool GetMouseButtonPressed(int btn) {
            return curr_button_state[btn]==PRESS;
        }

        [[nodiscard]] vec2 GetCursorOffset() const{
            return (curr_cursor_pos-last_cursor_pos)*cursor_sensitivity;
        }
    };
}
#endif //WX_COMPONENTS_H
