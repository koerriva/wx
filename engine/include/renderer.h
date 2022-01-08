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

#include "ecs.h"
#include "components.h"

namespace wx {
    using namespace std;
    using namespace glm;

    class Assets {
    public:
        static uint32_t UnitQuad();
        static vector<entity_id> LoadModelFromGLTF(const char *filename);
    };

    class TextureLoader {
    public:
        static uint32_t Load(const unsigned char *buffer, int len, ivec2 filter, ivec2 warp);

        static shadow_map_t LoadDepthMap(uint32_t width, uint32_t height);

        static shadow_map_t LoadDepthCubeMap(uint32_t width, uint32_t height);
    };

    enum ShaderType {
        VERTEX_SHADER = 1, GEOMETRY_SHADER, FRAGMENT_SHADER
    };

    class ShaderProgram {
    private:
        uint32_t value;
        static unordered_map<string, int> uniforms;

        static unsigned int CreateShader(GLuint type, const char *source);

    public:
        operator uint32_t() { return value; }

        ShaderProgram operator=(uint32_t v) {
            ShaderProgram s;
            s.value = v;
            return s;
        }

        static uint32_t LoadShader(const char *name, bool geom = false);

        static void Bind(uint32_t pid);

        static void Unbind();

        static void Cleanup(uint32_t pid);

        static int FindUniformLocation(uint32_t pid, const string &name);

        static void SetFloat(uint32_t pid, const string &name, float value);

        static void SetMat4(uint32_t pid, const string &name, float *value);

        static void SetVec4(uint32_t pid, const string &name, float *value);

        static void SetVec3(uint32_t pid, const string &name, float *value);

        static void SetVec2(uint32_t pid, const string &name, float *value);

        static void SetInt(uint32_t pid, const string &name, int value);

        static void SetAttenuation(uint32_t pid, const string &_name, Light::attenuation_t value);

        static void SetLight(uint32_t pid, const string &_name, vector<Light> &lights);
    };

    class Renderer {
    private:
        bool WIREFRAME_MODE = false;
        bool SHADER_MODE = true;

        float frame_time = 0;
        uint32_t frame_count = 0;

        light_t* view_light;
        bool is_light_view = false;
    public:

        void Render(const Window *window, vector<model_t> &models, vector<light_t> &lights, float delta);

        void Render(const Window *window, const Camera *camera, vector<model_t> &models, vector<light_t> &lights,
                    canvas_t canvas, float delta);
    };

    class Debug {
    private:
        uint32_t shaderProgram;
        vector<float> vertices;
        GLuint VAO{}, VBO{};
    public:
        Debug();
        void PrintScreen(vec2 pos, const wchar_t *text, vec3 color);
    };
}

#endif //WX_RENDERER_H
