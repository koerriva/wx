#version 330

in vec2 v_TexCoord;
out vec4 FragColor;

uniform sampler2D texture0;
uniform vec3 color;

void main() {
    vec3 diffuse = texture(texture0,v_TexCoord);
    FragColor = vec4(diffuse*color, 1.0);
}