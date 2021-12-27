#version 330

in vec2 v_TexCoord;
out vec4 FragColor;

uniform sampler2D color_texture;
uniform vec3 color;

void main() {
    float r = texture(color_texture,v_TexCoord).r;
    vec4 samplerC = vec4(1.0,1.0,1.0,r);
    FragColor = vec4(color, 1.0)*samplerC;
}
