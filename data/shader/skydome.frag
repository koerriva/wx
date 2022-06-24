#version 330 core

in vec3 pos;
out vec4 color;

void main()
{
    vec3 base_color = vec3(0.18,0.27,0.47);
    float r = -0.00022 * (pos.y-2800) + base_color.x;
    float g = -0.00025 * (pos.y-2800) + base_color.y;
    float b = -0.00019 * (pos.y-2800) + base_color.z;

    color = vec4(r,g,b,1.0);
}