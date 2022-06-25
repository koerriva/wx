#version 330 core
layout (location = 0) in vec3 position;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec3 world_Position;

void main()
{
    gl_Position = P * V * M * vec4(position, 1.0);

    world_Position = (M * vec4(position, 1.0)).xyz;
}