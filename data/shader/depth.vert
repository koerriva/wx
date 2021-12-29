#version 330

layout (location = 0) in vec3 position;
layout (location = 3) in uvec4 joint;
layout (location = 4) in vec4 weight;

uniform mat4 PV;
uniform mat4 M;
uniform int use_skin;
uniform mat4 JointMat[64];

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
    gl_Position = PV*model*vec4(position, 1.0);
}