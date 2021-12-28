#version 330

in vec2 v_TexCoord;
out vec4 FragColor;

uniform sampler2D texture0;
uniform vec3 color;

uniform int type;

uniform float near_plane;
uniform float far_plane;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

float unpackDepth(const in vec4 rgbaDepth) {
    const vec4 bitShift = vec4(1.0, 1.0/256.0, 1.0/(256.0*256.0), 1.0/(256.0*256.0*256.0));
    float depth = dot(rgbaDepth, bitShift);
    return depth;
}

void main() {
    if(type==0){
        float depthC = texture(texture0,v_TexCoord).r;
        FragColor = vec4(vec3(LinearizeDepth(depthC) / far_plane), 1.0); // perspective
    }
    if(type==1){
        vec4 rgbaDepth = texture(texture0,v_TexCoord);
        float depthC = unpackDepth(rgbaDepth);
        float d = LinearizeDepth(depthC) / far_plane;
        FragColor = vec4(vec3(d), 1.0); // perspective
    }
    if(type==2){
        float depthC = texture(texture0,v_TexCoord).r;
        FragColor = vec4(vec3(depthC*0.1-0.5,depthC*0.1,depthC*0.1), 1.0); // orthographic
    }
}