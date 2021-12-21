#version 330
const float pi = 3.1415926;

out vec4 FragColor;

in vec2 v_TexCoord;
in vec3 v_Color;
in vec3 v_WorldPos;
in vec3 v_Normal;

vec3 light_pos = vec3(5.0,5.0,0.0);
vec3 light_color = vec3(1.0);
vec3 ambient = vec3(0.3)*light_color;

uniform vec4 base_color;
uniform float metallic_factor;
uniform float roughness_factor;

uniform sampler2D texture0;

void main(){
    vec4 tex_color = texture(texture0,v_TexCoord);

    vec3 light_dir = normalize(light_pos-v_WorldPos);

    vec3 diffuse = max(dot(v_Normal,light_dir),0.0)*light_color;
    FragColor = base_color*vec4((ambient+diffuse)*tex_color.rgb,1.0);
}