#version 330 core
out vec4 FragColor;

in vec3 v_TexCoord;

uniform samplerCube skyboxMap;

void main()
{
    FragColor = texture(skyboxMap,v_TexCoord);
//    FragColor = vec4(1.0);
}