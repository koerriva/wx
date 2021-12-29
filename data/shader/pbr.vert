#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in uvec4 joint;
layout (location = 4) in vec4 weight;

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

void main(){

    mat4 model = M;
    if(use_skin==1){
        mat4 SkinMat =
        weight.x * JointMat[int(joint.x)] +
        weight.y * JointMat[int(joint.y)] +
        weight.z * JointMat[int(joint.z)] +
        weight.w * JointMat[int(joint.w)];

        model = SkinMat*M;
    }

    v_TexCoord = texcoord;
    v_Normal = mat3(transpose(inverse(model))) * normal;
    v_WorldPos = (model*vec4(position,1.0)).xyz;

    for(int i=0;i<5;i++){
        v_LightWorldPos[i] = LightPV[i]*M*vec4(position,1.0);
    }

    gl_Position = P*V*model*vec4(position,1.0);
}