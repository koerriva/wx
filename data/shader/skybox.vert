#version 330
layout (location = 0) in vec3 position;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec3 v_TexCoord;

void main()
{
    v_TexCoord = position;
//    gl_Position = P*V*M*vec4(position, 1.0);
    gl_Position = P*mat4(mat3(V))*vec4(position, 1.0);
}