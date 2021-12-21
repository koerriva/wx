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

uniform sampler2D base_color_sampler;
uniform sampler2D metallic_roughness_sampler;

void main(){
    vec4 base_color = texture(base_color_sampler,v_TexCoord);
    vec4 metallic_roughness_color = texture(metallic_roughness_sampler,v_TexCoord);

    vec3 light_dir = normalize(light_pos-v_WorldPos);

    vec3 diffuse = max(dot(v_Normal,light_dir),0.0)*light_color;
    FragColor = vec4(v_Color*(ambient+diffuse),1.0);
}