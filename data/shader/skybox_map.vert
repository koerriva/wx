#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;

uniform mat4 M;

//---------UNIFORM------------
uniform vec3 sun_pos;//sun position in world space
//uniform mat4 mvp;
uniform mat3 rot_stars;//rotation matrix for the stars
//---------OUT------------
out vec3 v_pos;
out vec3 v_sun_norm;
out vec3 v_star_pos;
out vec3 v_color;

void main()
{
    gl_Position = M * vec4(position, 1.0);

    v_pos = position;

    //Sun pos being a constant vector, we can normalize it in the vshader
    //and pass it to the fshader without having to re-normalize it
    v_sun_norm = normalize(sun_pos);

    //And we compute an approximate star position using the special rotation matrix
    v_star_pos = rot_stars * normalize(v_pos);
}