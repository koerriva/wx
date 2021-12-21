#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in vec3 color;

uniform float time;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec2 v_TexCoord;
out vec3 v_Color;
out vec3 v_WorldPos;
out vec3 v_Normal;

void main(){
    gl_Position = P*V*M*vec4(position,1.0);
    v_TexCoord = texcoord;
    v_Color = color;
    v_Normal = mat3(transpose(inverse(M))) * normal;
    v_WorldPos = (M*vec4(position,1.0)).xyz;
}