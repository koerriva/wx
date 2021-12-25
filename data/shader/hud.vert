#version 330

layout (location=0) in vec4 vertex;

out vec2 v_TexCoord;
uniform mat4 P;
uniform mat4 M;
void main() {
    gl_Position = P*M*vec4(vertex.xy,0.0,1.0);
    v_TexCoord = vertex.zw;
}