#version 330
const float PI = 3.1415926;

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
    vec3 color;
    vec3 position;//世界空间坐标
    vec3 direction;//方向
    float intensity;
    float cutoff;
    Attenuation att;
};

out vec4 FragColor;

in vec2 v_TexCoord;
in vec3 v_Color;
in vec3 v_WorldPos;
in vec3 v_Normal;

uniform int light_num;
uniform Light lights[20];

uniform vec3 cameraPos;

//反射颜色 - 可以换成贴图需要从sRGB 转换到 linerRGB
uniform vec4  albedo;
uniform int has_albedo_texture;
uniform sampler2D albedo_texture;
//金属度 可以换成贴图
uniform float metallic;
//粗糙度 可以换成贴图
uniform float roughness;
uniform int has_metallic_roughness_texture;
uniform sampler2D metallic_roughness_texture;

//环境光遮蔽 - 可以换成贴图需要从sRGB 转换到 linerRGB
uniform float ao;

//uniform sampler2D texture0;

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
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

void main(){
    vec4 albedo_factor = albedo;
    if(has_albedo_texture==1){
        vec4 c = texture(albedo_texture,v_TexCoord);
        albedo_factor = vec4(pow(c.rgb,vec3(2.2)),c.a);
    }

    float metallic_factor = metallic;
    float roughness_factor = roughness;
    if(has_metallic_roughness_texture==1){
        vec2 mr = texture(metallic_roughness_texture,v_TexCoord).bg;
        metallic_factor = mr.x;
        roughness_factor = mr.y;
    }

    vec3 N = normalize(v_Normal);
    vec3 V = normalize(cameraPos - v_WorldPos); //出射光线

    vec3 Lo = vec3(0.0); //出射光线的辐射率
    for(int i=0;i<light_num;i++){
        Light light = lights[i];

        vec3 L = normalize(light.position - v_WorldPos); //入射光方向
        if(light.type==DIRECTIONAL_LIGHT){
            L = normalize(light.direction);
        }
        vec3 H = normalize(V+L);

        vec3 radiance = light.color * light.intensity;

        if(light.type==POINT_LIGHT){
            float distance = length(light.position - v_WorldPos);

            //        float attenuation = light.att.exponent / (distance * distance);//衰减
            //        vec3 radiance = (light.color * light.intensity) * attenuation;

            float attenuationInv = light.att.constant + light.att.linear * distance + light.att.exponent * (distance * distance);//衰减
            radiance /= attenuationInv;
        }
        if(light.type==SPOT_LIGHT){
            vec3 formLightDir = -L;
            vec3 coneLightDir = normalize(light.direction);
            float ccosTheta = dot(formLightDir,coneLightDir);
            float attenuation = 1.0 - (1.0-ccosTheta)/(1.0-light.cutoff);
            if(ccosTheta > light.cutoff){
                radiance *= attenuation;
            }else{
                radiance *= 0.0f;
            }
        }

        vec3 F0 = vec3(0.04);//非金属材质默认0.04
        F0 = mix(F0,albedo_factor.rgb,metallic_factor);
        vec3 F = fresnelSchlick(max(dot(H,V),0.0),F0);

        float NDF = DistributionGGX(N,H,roughness_factor);
        float G = GeometrySmith(V,L,H,roughness_factor);

        vec3 nominator = NDF * G * F;
        float demoinator = 4.0 * max(dot(N,V),0.0) * max(dot(N,L),0.0) + 0.001;//加0.001防止除零
        vec3 specular = nominator/demoinator;

        vec3 kS = F; //光被反射的比例
        vec3 kD = vec3(1.0) - kS;//光被折射的比例
        kD *= 1.0 - metallic_factor;

        float NdotL = max(dot(N,L),0.0);
        Lo += (kD*albedo_factor.rgb/PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * albedo_factor.rgb * ao;
    vec3 color = ambient + Lo;
    color = color / (color + vec3(1.0));//LDR to HDR
    color = pow(color, vec3(1.0/2.2)); // HDR to Gamma2
    FragColor = vec4(color,1.0);
}