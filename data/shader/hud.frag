#version 330

in vec2 v_TexCoord;
out vec4 FragColor;

uniform sampler2D texture0;
uniform vec3 color;

float near_plane = 0.1;
float far_plane = 100;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main() {
    float depthC = texture(texture0,v_TexCoord).r;
    vec4 samplerC = vec4(vec3(depthC),1.0);
    FragColor = vec4(samplerC);

    FragColor = vec4(vec3(LinearizeDepth(depthC) / far_plane), 1.0); // perspective
    // color = vec4(vec3(depthValue), 1.0); // orthographic
}