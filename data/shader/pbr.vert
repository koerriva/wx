#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in uvec4 joint;
layout (location = 4) in vec4 weight;
layout (location = 5) in vec4 tangent;

uniform float time;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 LightPV[5];
uniform int use_skin;
uniform mat4 JointMat[64];

out vec2 v_TexCoord;
out vec3 v_WorldPos;
out vec3 v_Normal;
out vec4 v_LightWorldPos[5];
out mat3 TBN;

void main(){

    mat4 model = M;
    if(use_skin==1){
        mat4 SkinMat =
        weight.x * JointMat[int(joint.x)] +
        weight.y * JointMat[int(joint.y)] +
        weight.z * JointMat[int(joint.z)] +
        weight.w * JointMat[int(joint.w)];

        model = SkinMat;
    }

    v_TexCoord = texcoord;
    v_Normal = mat3(transpose(inverse(model))) * normal;
    v_WorldPos = (model*vec4(position,1.0)).xyz;

    for(int i=0;i<5;i++){
        v_LightWorldPos[i] = LightPV[i]*model*vec4(position,1.0);
    }

    vec3 bitangent = cross(normal,tangent.xyz) * tangent.w;
    vec3 T = normalize(vec3(model * vec4(tangent.xyz,   0.0)));
    vec3 B = normalize(vec3(model * vec4(bitangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(normal,    0.0)));
    TBN = mat3(T, B, N);

    gl_Position = P*V*model*vec4(position,1.0);
}