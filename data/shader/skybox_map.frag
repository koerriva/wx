#version 330 core
const float PI = 3.141592653589793;
in vec4 FragPos;

//---------IN------------
in vec3 pos;
in vec3 sun_norm;
in vec3 star_pos;
//---------UNIFORM------------
uniform sampler2D tint;//the color of the sky on the half-sphere where the sun is. (time x height)
uniform sampler2D tint2;//the color of the sky on the opposite half-sphere. (time x height)
uniform sampler2D sun;//sun texture (radius x time)
uniform sampler2D moon;//moon texture (circular)
uniform sampler2D clouds1;//light clouds texture (spherical UV projection)
uniform sampler2D clouds2;//heavy clouds texture (spherical UV projection)
uniform float weather;//mixing factor (0.5 to 1.0)
uniform float time;
//---------OUT------------
out vec3 color;

//---------NOISE GENERATION------------
//Noise generation based on a simple hash, to ensure that if a given point on the dome
//(after taking into account the rotation of the sky) is a star, it remains a star all night long
float Hash(float n){
    return fract((1.0 + sin(n)) * 415.92653);
}
float Noise3d(vec3 x){
    float xhash = Hash(round(400*x.x) * 37.0);
    float yhash = Hash(round(400*x.y) * 57.0);
    float zhash = Hash(round(400*x.z) * 67.0);
    return fract(xhash + yhash + zhash);
}
vec3 sphericl(vec3 p){
    float x=p.x,y=p.y,z=p.z;
    float xx = x*x;float yy = y*y;float zz = z*z;

    x = x*sqrt(1.f-yy/2.f-zz/2.f+yy*zz/3.f);
    y = y*sqrt(1.f-zz/2.f-xx/2.f+zz*xx/3.f);
    z = z*sqrt(1.f-xx/2.f-yy/2.f+xx*yy/3.f);
    return vec3(x,y,z);
}
void main()
{
    vec3 pos_norm = sphericl(pos);
//    vec3 pos_norm = pos;
    color = vec3(1.f);

    color = mix(vec3(0.77,0.77,0.899),vec3(0.2,0.3,mix(0.3,0.7,abs(pos_norm.y))),abs(pos_norm.y));
}