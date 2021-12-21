#version 330
const float pi = 3.1415926;

out vec4 FragColor;

in vec2 v_TexCoord;
in vec3 v_Color;
in vec3 v_WorldPos;
in vec3 v_Normal;

vec3 light_pos = vec3(5.0,5.0,0.0);
vec3 light_color = vec3(1.0);
vec3 ambient = vec3(0.3)*light_color;

uniform float time;
uniform vec3 view_pos;
uniform sampler2D base_color_sampler;
uniform sampler2D metallic_roughness_sampler;

float near = 0.01;
float far  = 1000.0;
float width = 1280;
float height = 720;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // back to NDC
    return (2.0 * near * far) / (far + near - z * (far - near));
}

vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec2 mod289(vec2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute(vec3 x) { return mod289(((x*34.0)+1.0)*x); }

float snoise(vec2 v) {
    const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
    0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
    -0.577350269189626,  // -1.0 + 2.0 * C.x
    0.024390243902439); // 1.0 / 41.0
    vec2 i  = floor(v + dot(v, C.yy) );
    vec2 x0 = v -   i + dot(i, C.xx);
    vec2 i1;
    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;
    i = mod289(i); // Avoid truncation effects in permutation
    vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
    + i.x + vec3(0.0, i1.x, 1.0 ));

    vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
    m = m*m ;
    m = m*m ;
    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;
    m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
    vec3 g;
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

//由近及远，由上到下
void main(){
    vec4 color = vec4(v_Color,1.0);

    float depth = gl_FragCoord.z;
    float height = (v_WorldPos.y+1.0)/6.0;
    color.a = height*height*depth;

    //描边
    vec3 vier_dir = normalize(view_pos-v_WorldPos);
    float angle = max(dot(v_Normal,vier_dir),0.0);
    if(angle<0.4){
        color.rgb += angle*angle;
        color.a += angle*0.5;
    }

    //水面
    if(v_WorldPos.y>=-1.0&&v_WorldPos.y<=-0.95){
        float factor = max(dot(v_Normal,vec3(0,1,0)),0.0);
        color.a = factor*factor*0.2;
    }
    FragColor = color;
}

//void main(){
//    vec4 base_color = texture(base_color_sampler,v_TexCoord);
//
//    vec3 view_dir = normalize(view_pos-v_WorldPos);
//    float hit_color = abs(dot(v_Normal,view_dir));
//
//    if(hit_color<0.4){
//        //边缘
//        FragColor = vec4(v_Color+hit_color*hit_color,1.0);
//    }else if(hit_color<=1.0&&hit_color>0.98){
//        float a = hit_color*hit_color;
//        FragColor = vec4(v_Color+a*0.25,a);
//    }else{
//        FragColor = vec4(v_Color,0);
//    }
//
//    if(v_WorldPos.y==-1.0){
//        vec2 st = v_TexCoord.xy;
//
//        vec3 color = v_Color;
//
//        vec2 pos = vec2(st*3.);
//
//        vec2 vel = vec2(time*.05);
//
//        float r = snoise(pos+vel)*.25+.25;
//
//        float a = snoise(pos*vec2(cos(time*0.15),sin(time*0.1))*0.1)*3.1415;
//        vel = vec2(cos(a),sin(a));
//        r += snoise(pos+vel)*.25+.25;
//
//        color += vec3(smoothstep(.1,.25,fract(r)));
//        FragColor = vec4(1.0-color,r);
//    }
//}