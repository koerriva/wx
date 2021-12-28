#version 330

layout (location = 0) in vec3 position;

uniform mat4 PV;
uniform mat4 M;

void main(){
    gl_Position = PV*M*vec4(position, 1.0);
}