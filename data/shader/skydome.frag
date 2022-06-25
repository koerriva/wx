#version 330 core

in vec3 world_Position;

uniform float radius;//半径
uniform vec3 sun_pos;
uniform float time;
uniform vec3 camera_pos;//相机世界坐标

out vec4 color;

//noise
//	Simplex 3D Noise 
//	by Ian McEwan, Ashima Arts
//
vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
vec4 taylorInvSqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}

float snoise(vec3 v){ 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //  x0 = x0 - 0. + 0.0 * C 
  vec3 x1 = x0 - i1 + 1.0 * C.xxx;
  vec3 x2 = x0 - i2 + 2.0 * C.xxx;
  vec3 x3 = x0 - 1. + 3.0 * C.xxx;

// Permutations
  i = mod(i, 289.0 ); 
  vec4 p = permute( permute( permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients
// ( N*N points uniformly over a square, mapped onto an octahedron.)
  float n_ = 1.0/7.0; // N=7
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z *ns.z);  //  mod(p,N*N)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
}

#define bottom 100 //云层底部
#define top 200 //云层顶部
#define width 200 //云层xz平面

float getDensity(vec3 point){
    float noise = snoise(point*0.005);
    if(noise<0.4){
        return 0.0;
    }
    return noise;
}

vec4 getCloud(vec3 worldPos,vec3 cameraPos){
    vec3 direction = normalize(worldPos-cameraPos);
    vec3 step = direction * 1;
    vec4 colorSum = vec4(0);
    vec3 point = cameraPos;

    //ray marching

    //如果相机在云层下，将测试点移动到云层底部
    if(point.y<bottom){
        point += direction * (abs(bottom-cameraPos.y)/abs(direction.y));
    }
    //如果相机在云层上，将测试点移动到云层顶部
    if(top<point.y){
        point += direction * (abs(cameraPos.y-top)/abs(direction.y));
    }
    //如果目标像素遮挡了云层，则放弃测试
    float len1 = length(point-cameraPos);
    float len2 = length(worldPos-cameraPos);
    if(len2<len1){
        return vec4(0);
    }
    for(int i=0;i<200;i++){
        point += step;
        if(bottom>point.y||point.y>top||-width>point.x||point.x>width||-width>point.z||point.z>width){
            continue;
        }
        float density = getDensity(point);
        vec4 color = vec4(0.9,0.8,0.7,1.0) * density;//当前点颜色
        colorSum = colorSum + color * (1-colorSum.a); //累积色
    }

    return colorSum;
}

void main()
{
    vec3 base_color = vec3(0.18,0.27,0.47);
    vec3 sun_color = vec3(1.0,1.0,0.87);

    float r = -0.00022 * (world_Position.y-radius);
    float g = -0.00025 * (world_Position.y-radius);
    float b = -0.00019 * (world_Position.y-radius);
    base_color = base_color+vec3(r,g,b);

    float sun_distance = distance(world_Position,sun_pos);
    float factor = 6000/(sun_distance*sun_distance);
    sun_color = sun_color*factor;

    color = vec4(base_color+sun_color,1.0);

    factor = snoise((world_Position/vec3(radius))*1);

    vec4 cloud_color = getCloud(world_Position,camera_pos);

    color = vec4(color.rgb*(1.0-cloud_color.a)+cloud_color.rgb,1.0);
}