#version 430 core
const float PI = 3.141592653589793;

#define POINT_LIGHT 0
#define SPOT_LIGHT 1
#define DIRECTIONAL_LIGHT 2

struct Attenuation
{
    float constant;
    float linear;
    float exponent;
};

struct Light
{
    int type;
    int state;
    vec3 color;
    vec3 position;//世界空间坐标
    vec3 direction;//方向
    float intensity;
    float cutoff;
    int has_shadow_map;
    float near_plane;
    float far_plane;
    int shadow_map_index;
    Attenuation att;
};

out vec4 FragColor;

in vec2 v_TexCoord;
in vec3 v_Color;
in vec3 v_WorldPos;
in vec3 v_Normal;
in vec4 v_LightWorldPos[5];
in mat3 TBN;

uniform int light_num;
uniform Light lights[20];

uniform vec3 cameraPos;

//反射颜色 - 可以换成贴图需要从sRGB 转换到 linerRGB
uniform vec4  albedo_factor;
uniform int has_albedo_texture;
uniform sampler2D albedo_texture;
//金属度 可以换成贴图
uniform float metallic_factor;
//粗糙度 可以换成贴图
uniform float roughness_factor;
uniform int has_metallic_roughness_texture;
uniform sampler2D metallic_roughness_texture;
//环境光遮蔽 - 可以换成贴图需要从sRGB 转换到 linerRGB
uniform float ao_factor;
uniform int has_occlusion_texture;
uniform sampler2D occlusion_texture;
uniform double occlusion_strength;
//法线贴图
uniform int has_normal_texture;
uniform sampler2D normal_texture;
uniform double normal_scale;

//平行光阴影贴图
uniform sampler2D shadowMap[5];
//点光源阴影贴图
uniform samplerCube shadowCubeMap[5];

//天空盒
uniform int has_skybox;
uniform samplerCube skyboxMap;

//菲涅尔函数-计算镜面反射和漫反射系数
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

//正态分布函数
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

//几何遮蔽函数
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;// Epic suggests using this roughness remapping for analytic lights.

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float random(vec3 seed, int i){
    vec4 seed4 = vec4(seed,i);
    float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot_product) * 43758.5453);
}

vec2 poissonDisk[16] = vec2[](
vec2( -0.94201624, -0.39906216 ),
vec2( 0.94558609, -0.76890725 ),
vec2( -0.094184101, -0.92938870 ),
vec2( 0.34495938, 0.29387760 ),
vec2( -0.91588581, 0.45771432 ),
vec2( -0.81544232, -0.87912464 ),
vec2( -0.38277543, 0.27676845 ),
vec2( 0.97484398, 0.75648379 ),
vec2( 0.44323325, -0.97511554 ),
vec2( 0.53742981, -0.47373420 ),
vec2( -0.26496911, -0.41893023 ),
vec2( 0.79197514, 0.19090188 ),
vec2( -0.24188840, 0.99706507 ),
vec2( -0.81409955, 0.91437590 ),
vec2( 0.19984126, 0.78641367 ),
vec2( 0.14383161, -0.14100790 )
);
float CalcDirLightShadow(sampler2D shadowMap,vec4 fragPosLightSpace,float bias)
{
    // 执行透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 变换到[0,1]的范围
    projCoords = projCoords * 0.5 + 0.5;
    // 取得当前片段在光源视角下的深度
    float currentDepth = projCoords.z;

    float shadow = 0.0;

    //PCF 多重采样
//    vec2 texSize = 1.0/textureSize(shadowMap,0);//0级纹理,原始大小
//    for(int x = -1; x <= 1; ++x)
//    {
//        for(int y = -1; y <= 1; ++y)
//        {
//            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texSize).r;
//            // 检查当前片段是否在阴影中
//            shadow += currentDepth - bias > pcfDepth ? 0.91 : 0.0;
//        }
//    }
//    shadow /= 9.0;

    //Poisson Sampling
    for(int i=0;i<4;i++){
        float psDepth = texture(shadowMap, projCoords.xy + poissonDisk[i]/700.0).r;
        shadow += currentDepth - bias > psDepth ? 0.24 : 0.0;
    }

    //Stratified Poisson Sampling
//    for(int i=0;i<4;i++){
//        int index = int(16.0*random(fragPosLightSpace.xyz, i))%16;
//        float psDepth = texture(shadowMap, projCoords.xy + poissonDisk[index]/700.0).r;
//        shadow += currentDepth - bias > psDepth ? 0.2 : 0.0;
//    }

    // 超出视锥区忽略
    if(projCoords.z>1.0){
        return 0.0;
    }

    return shadow;
}

float CalcSpotLightShadow(sampler2D shadowMap,vec4 fragPosLightSpace,float bias)
{
    // 执行透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 变换到[0,1]的范围
    projCoords = projCoords * 0.5 + 0.5;
    // 取得当前片段在光源视角下的深度
    float currentDepth = projCoords.z;

    float shadow = 0.0;

    //PCF 多重采样
//    vec2 texSize = 1.0/textureSize(shadowMap,0);//0级纹理,原始大小
//    for(int x = -1; x <= 1; ++x)
//    {
//        for(int y = -1; y <= 1; ++y)
//        {
//            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texSize).r;
//            // 检查当前片段是否在阴影中
//            shadow += currentDepth - bias > pcfDepth ? 0.91 : 0.0;
//        }
//    }
//    shadow /= 9.0;

    //Poisson Sampling
    for(int i=0;i<4;i++){
        float psDepth = texture(shadowMap, projCoords.xy + poissonDisk[i]/700.0).r;
        shadow += currentDepth - bias > psDepth ? 0.21 : 0.0;
    }

    //Stratified Poisson Sampling
//    for(int i=0;i<4;i++){
//        int index = int(16.0*random(fragPosLightSpace.xyz, i))%16;
//        float psDepth = texture(shadowMap, projCoords.xy + poissonDisk[index]/700.0).r;
//        shadow += currentDepth - bias > psDepth ? 0.2 : 0.0;
//    }

    // 超出视锥区忽略
//    if(projCoords.z>1.0){
//        return 0.0;
//    }

    return shadow;
}

vec3 sampleOffsetDirections[20] = vec3[]
(
vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);
float CalcPointLightShadow(samplerCube shadowMap,vec3 fragPos,vec3 lightPos,float nfar,float bias_n)
{
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);

    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;
    float viewDistance = length(cameraPos - fragPos);
//    float diskRadius = 0.05;
    float diskRadius = (1.0 + (viewDistance / nfar)) / 25.0;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(shadowMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= nfar;   // Undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
        shadow += 0.95;
    }
    shadow /= float(samples);

    return shadow;
}

vec3 lerp(vec3 prev,vec3 next,float t){
    return prev+(next-prev)*t;
}

void main(){
    vec4 albedo = albedo_factor;
    if(has_albedo_texture==1){
        vec4 c = texture(albedo_texture,v_TexCoord);
        albedo = vec4(pow(c.rgb,vec3(2.2)),c.a);
    }

    float metallic = metallic_factor;
    float roughness = roughness_factor;
    if(has_metallic_roughness_texture==1){
        vec2 mr = texture(metallic_roughness_texture,v_TexCoord).bg;
        metallic = mr.x;
        roughness = mr.y;
    }

    vec3 occlusion = vec3(ao_factor);
    if(has_occlusion_texture==1){
        occlusion = texture(occlusion_texture,v_TexCoord).rrr;
        occlusion = lerp(vec3(ao_factor),vec3(ao_factor)*occlusion,float(occlusion_strength));
    }

    vec3 normal = v_Normal;
    if(has_normal_texture==1){
        normal = texture(normal_texture,v_TexCoord).rgb;
        normal = (normal * 2.0f - 1.0f) * vec3(normal_scale,normal_scale,1.0f);
        normal = TBN * normal;
    }

    vec3 N = normalize(normal);
    vec3 V = normalize(cameraPos - v_WorldPos); //出射光线
    vec3 Lo = vec3(0.0); //出射光线的辐射率
    vec3 F0 = vec3(0.04);//非金属材质默认0.04
    F0 = mix(F0,albedo.rgb,metallic);

    float shadow_factor = 1.0;

    for(int i=0;i<light_num;i++){
        Light light = lights[i];
        if(light.state==0)continue;

        vec3 L = normalize(light.position - v_WorldPos); //入射光方向
        vec3 H = normalize(V+L); //半高
        if(light.type==DIRECTIONAL_LIGHT){
            L = normalize(-light.direction);
            H = V;
        }
        vec3 radiance = light.color * light.intensity;
        //计算阴影
        float bias = max(0.005 * (1.0 - dot(normal, L)), 0.005);//shadow bias
        if(light.type==POINT_LIGHT){
            float distance = length(light.position - v_WorldPos);

            //        float attenuation = light.att.exponent / (distance * distance);//衰减
            //        vec3 radiance = (light.color * light.intensity) * attenuation;

            float attenuationInv = light.att.constant + light.att.linear * distance + light.att.exponent * (distance * distance);//衰减
            radiance /= attenuationInv;

            if(light.has_shadow_map==1){
//                float bias = 0.001;
                float shadow = CalcPointLightShadow(shadowCubeMap[light.shadow_map_index],v_WorldPos,light.position,light.far_plane,bias);
                radiance *= 1.0 - shadow;
            }
        }
        if(light.type==SPOT_LIGHT){
            vec3 formLightDir = -L;
            vec3 coneLightDir = normalize(light.direction);
            float ccosTheta = dot(formLightDir,coneLightDir);
            float attenuation = 1.0 - (1.0-ccosTheta)/(1.001-light.cutoff);
            if(ccosTheta > light.cutoff){
                radiance *= attenuation;
            }else{
                radiance *= 0.0f;
            }

            if(light.has_shadow_map==1){
//                float bias = 0.001;
                float shadow = CalcSpotLightShadow(shadowMap[light.shadow_map_index],v_LightWorldPos[light.shadow_map_index],bias);
                radiance *= 1 - shadow;
            }
        }
        if(light.type==DIRECTIONAL_LIGHT&&light.has_shadow_map==1){
//            float bias = 0.001;
            float shadow = CalcDirLightShadow(shadowMap[light.shadow_map_index],v_LightWorldPos[light.shadow_map_index],bias);
            radiance *= 1 - shadow;
        }

        // Calculate Fresnel term for direct lighting.
        vec3 F = fresnelSchlick(max(dot(H,V),0.0),F0);
        // Calculate normal distribution for specular BRDF.
        float NDF = DistributionGGX(N,H,roughness);
        // Calculate geometric attenuation for specular BRDF.
        float G = GeometrySmith(V,L,H,roughness);

        vec3 kS = F; //光被反射的比例
        vec3 kD = vec3(1.0) - kS;//光被折射的比例
        kD *= 1 - metallic;
        vec3 diffuseBRDF = kD * albedo.rgb;

        vec3 specularBRDF = (NDF * G * F)/max(4.0 * max(dot(N,V),0.0) * max(dot(N,L),0.0),0.00000001);

        float NdotL = max(dot(N,L),0.0);
        Lo += (diffuseBRDF/PI + specularBRDF) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03);
    if(has_skybox==1){
        vec3 I = normalize(v_WorldPos - cameraPos);
        vec3 R = reflect(I, normalize(v_Normal));
        ambient = texture(skyboxMap, R).rgb;
    }
    vec3 color = ambient * albedo.rgb * occlusion + Lo;
    color = color / (color + vec3(1.0));//HDR to LDR
    color = pow(color, vec3(1.0/2.2)); // LDR to Gamma2

    FragColor = vec4(color,1.0);
}