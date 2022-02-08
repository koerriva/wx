#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 faceMatrices[6];

out vec4 FragPos; // FragPos from GS (output per emitvertex)

in vec3 v_pos[3];
in vec3 v_sun_norm[3];
in vec3 v_star_pos[3];

out vec3 pos;
out vec3 sun_norm;
out vec3 star_pos;

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face; // built-in variable that specifies to which face we render.
        for(int i = 0; i < 3; ++i) // for each triangle's vertices
        {
            FragPos = gl_in[i].gl_Position;
            gl_Position = faceMatrices[face] * FragPos;

            pos = v_pos[i];
            sun_norm = v_sun_norm[i];
            star_pos = v_star_pos[i];

            EmitVertex();
        }
        EndPrimitive();
    }
}