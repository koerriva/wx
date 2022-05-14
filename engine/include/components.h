//
// Created by koerriva on 2022/1/8.
//

#ifndef WX_COMPONENTS_H
#define WX_COMPONENTS_H

#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <variant>

typedef uint32_t entity_id;

typedef uint32_t VAO;
typedef uint32_t TEXTURE;

namespace wx
{
    using namespace glm;

    struct MainCamera
    {
    };
    struct Camera
    {
        vec3 position{0.f, 0.f, 0.f};
        vec3 front{0.f, 0.f, -1.f};
        vec3 up{0.f, 1.f, 0.f};
        vec3 right{1.f, 0.f, 0.f};
        float pitch = 0;
        float yaw = -90;

        [[nodiscard]] quat GetRotation() const
        {
            return quat{vec3(radians(yaw), radians(pitch), radians(0.f))};
        }
    };

    struct Transform
    {
        vec3 position{0.0f};
        quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
        vec3 scale{1.0f};

        [[nodiscard]] mat4 GetLocalMatrix() const
        {
            mat4 T = translate(mat4{1.0f}, position);
            mat4 R = mat4_cast(rotation);
            mat4 S = glm::scale(mat4{1.0f}, scale);
            return T * R * S;
        }

        //        [[nodiscard]] mat4 GetGlobalMatrix() const {
        //            return matrix;
        //            mat4 local = GetLocalMatrix();
        //
        //            if(has_parent){
        //                return parent->GetGlobalMatrix()*local;
        //            }else{
        //                return local;
        //            }
        //        }
    };

    struct AnimatedTransform
    {
        vec3 position{0.0f};
        quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
        vec3 scale{1.0f};

        [[nodiscard]] mat4 GetLocalMatrix() const
        {
            mat4 T = translate(mat4{1.0f}, position);
            mat4 R = mat4_cast(rotation);
            mat4 S = glm::scale(mat4{1.0f}, scale);
            return T * R * S;
        }

        static AnimatedTransform From(Transform *pTransform)
        {
            return AnimatedTransform{.position = pTransform->position, .rotation = pTransform->rotation, .scale = pTransform->scale};
        }
    };

    typedef struct material_t
    {
        vec4 albedo_factor{1.f};
        int has_albedo_texture = 0;
        uint32_t albedo_texture;

        float ao = 1.0;
        int has_occlusion_texture = 0;
        uint32_t occlusion_texture;
        double occlusion_strength = 1.0;

        float metallic_factor = 1.0;
        float roughness_factor = 1.0;
        int has_metallic_roughness_texture = 0;
        uint32_t metallic_roughness_texture;

        int has_normal_texture = 0;
        uint32_t normal_texture;
        double normal_scale = 1.0;
    } material_t;

    typedef struct shadow_map_t
    {
        uint32_t fbo;
        uint32_t texture;
        uint32_t width;
        uint32_t height;
    } shadow_map_t;

    typedef struct cubemap_t
    {
        uint32_t fbo;
        uint32_t texture;
        uint32_t width;
        uint32_t height;
    } cubemap_t;

    struct CastShadow
    {
    };
    struct ReceiveShadow
    {
    };

    struct Light
    {
        enum Type
        {
            point = 0,
            spot,
            directional,
            ambient
        };

        typedef struct
        {
            float constant;
            float linear;
            float exponent;
        } attenuation_t;

        Type type;
        int state; //0-关闭,1-开启
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

    struct Joint
    {
    };
#define MAX_JOINT_COUNT 64
    struct Skin
    {
        std::string name;
        int joints_count = 0;
        entity_id joints[MAX_JOINT_COUNT]{};
        mat4 inverse_bind_matrices[MAX_JOINT_COUNT]{};
    };

    struct Mesh
    {
        std::string name;
        struct primitive_t
        {
            VAO vao{};
            int vertices_count = 0;
            int indices_count = 0;
            uint32_t indices_type = 0x1403;
            material_t material;
        };
        std::vector<primitive_t> primitives;
    };

    typedef struct keyframe_t
    {
        float time = 0;
        vec3 translation{0};
        quat rotation{1, 0, 0, 0};
        vec3 scale{1};
    } keyframe_t;

#define MAX_ANIMATION_COUNT 100
#define MAX_CHANNEL_COUNT 240
#define MAX_KEYFRAME_COUNT 120

    typedef struct channel_t
    {
        int keyframe_count = 0;
        keyframe_t keyframe[MAX_KEYFRAME_COUNT]{};
        int interpolation = 0; //0-liber,1-step,2-cubic
        bool has_translation = false;
        bool has_rotation = false;
        bool has_scale = false;
        //must have AnimatedTransform component
        ::entity_id target = 0;
    } channel_t;

    typedef struct animation_t
    {
        int channel_count = 0;
        channel_t channels[MAX_CHANNEL_COUNT]{};
        char name[45]{0};
    } animation_t;

    struct Canvas
    {
        VAO vao;
        TEXTURE texture;
        vec2 position{0};
        vec2 size{0};
    };

    struct HUD
    {
    };

    struct Animator
    {
        enum State
        {
            stop = 0,
            play,
            pause
        };
        enum PlayState
        {
            begin = 0,
            playing,
            end
        };

        float currTime[MAX_ANIMATION_COUNT] = {0};
        keyframe_t *prevFrame[MAX_ANIMATION_COUNT][MAX_CHANNEL_COUNT] = {nullptr};
        keyframe_t *nextFrame[MAX_ANIMATION_COUNT][MAX_CHANNEL_COUNT] = {nullptr};

        State state = stop; //0-stop,1-play,2-pause
        PlayState playState = end;
        bool loop = false;
        std::string playingAnimation;
        std::vector<animation_t> animations;

        void Play()
        {
            Play("");
        }

        void Play(const std::string &name, bool b_loop = true)
        {
            state = play;
            playState = begin;
            this->loop = b_loop;
            playingAnimation = name;
        }

        void Pause()
        {
            state = pause;
        }

        void Stop()
        {
            state = stop;
            playState = end;
            this->loop = false;
            playingAnimation = "";
        }
    };

    struct Skybox
    {
        cubemap_t cubemap;
        vec3 sun_pos;
        mat3 rot_stars;
        TEXTURE tint;        //the color of the sky on the half-sphere where the sun is. (time x height)
        TEXTURE tint2;       //the color of the sky on the opposite half-sphere. (time x height)
        TEXTURE sun;         //sun texture (radius x time)
        TEXTURE moon;        //moon texture (circular)
        TEXTURE clouds1;     //light clouds texture (spherical UV projection)
        TEXTURE clouds2;     //heavy clouds texture (spherical UV projection)
        float weather = 0.5; //mixing factor (0.5 to 1.0)
    };

    struct Spatial3d
    {
        std::string name;
        ::entity_id parent;
        std::vector<entity_id> children;
    };

    //resource
    struct VPMatrices
    {
        mat4 view{1.0f};
        mat4 project{1.0f};
        mat4 ortho{1.0f};
    };
    struct FrameState
    {
        float delta_time = 0.0f;
        float total_time = 0.0f;
        int total_count = 0.0f;
    };

    struct RenderState
    {
        enum RenderMode
        {
            Shader,
            Wireframe
        };
        RenderMode mode = Shader;
    };

    struct PBRShader
    {
        uint32_t id;
    };

    struct TerrainShader
    {
        uint32_t id;
    };

    struct FlatShader
    {
        uint32_t id;
    };

    struct FontShader
    {
        uint32_t id;
    };

    struct DepthShader
    {
        uint32_t id;
    };

    struct DepthCubeShader
    {
        uint32_t id;
    };

    struct SkyboxMapShader
    {
        uint32_t id;
    };

    struct SkyboxShader
    {
        uint32_t id;
    };

    struct SkydomeShader
    {
        uint32_t id;
    };

    struct WindowConfig
    {
        std::string title;
        int width = 1280;
        int height = 720;
        bool vSync = true;
    };

    struct InputState
    {
        const float cursor_sensitivity = 0.05;
        enum KeyCode
        {
            ESC = 256,
            ENTER,
            TAB,
            RIGHT = 262,
            LEFT,
            DOWN,
            UP,
            F1 = 290,
            F2,
            F3,
            F4,
            F5,
            F6,
            F7,
            F8,
            F9,
            F10,
            F11,
            F12,
            Num0 = 48,
            Num1,
            Num2,
            Num3,
            Num4,
            Num5,
            Num6,
            Num7,
            Num8,
            Num9,
            A = 65,
            B,
            C,
            D,
            E,
            F,
            G,
            H,
            I,
            J,
            K,
            L,
            M,
            N,
            O,
            P,
            Q,
            R,
            S,
            T,
            U,
            V,
            W,
            X,
            Y,
            Z
        };

        enum MouseCode
        {
            M_LEFT = 0,
            M_RIGHT = 1,
            M_MID = 2
        };

        enum KeyState
        {
            RELEASE = 0,
            PRESS,
            REPEAT, //GLFW_PRESS=1,GLFW_RELEASE=0
        };

        int last_key_state[349] = {0};
        int curr_key_state[349] = {0};

        int last_button_state[3] = {0};
        int curr_button_state[3] = {0};

        vec2 last_cursor_pos{0};
        vec2 curr_cursor_pos{0};

        bool GetKeyPressed(int key)
        {
            return last_key_state[key] == PRESS && curr_key_state[key] == RELEASE;
        }

        bool GetKeyDown(int key)
        {
            return curr_key_state[key] == PRESS;
        }

        bool GetKeyUp(int key)
        {
            return curr_key_state[key] == RELEASE;
        }

        bool GetMouseButtonPressed(int btn)
        {
            return curr_button_state[btn] == PRESS;
        }

        [[nodiscard]] vec2 GetCursorOffset() const
        {
            return (curr_cursor_pos - last_cursor_pos) * cursor_sensitivity;
        }
    };

    using NKData = std::variant<int, float, vec3, vec4>;
    struct NKMenu
    {
        std::string res;
        int resLen = 0;
        std::unordered_map<std::string,NKData> data;
    };

    struct NuklearContext
    {
        void *glfw;
        void *ctx;
    };

    //script
    struct LuaScript{
        std::string uri;
        bool cached = false;
        std::string code;
    };
}
#endif //WX_COMPONENTS_H
