#include <iostream>
#include <engine.h>
#include "DummyGame.h"

int main(int argc,char** argv) {
    system("chcp 65001");

    wx::Log::Init();
    wx::ResourceLoader::Init();

    WX_INFO("我的游戏引擎 0.2");
    wx::DummyGame game;
    wx::GameEngine engine("我的游戏引擎 0.2.0",1280,720, false,&game);
    engine.Run();

//    int w=4000,h=2000,comp=3;
//    auto* data = static_cast<unsigned char *>(malloc(w * h * comp));
//
//    module::Perlin perlin;
//    perlin.SetSeed(1234);
//    perlin.SetOctaveCount(7);
//    perlin.SetLacunarity(2.0);
//    perlin.SetPersistence(0.6);
//    perlin.SetFrequency(1);
//    perlin.SetNoiseQuality(NoiseQuality::QUALITY_BEST);
//
//    model::Sphere sphere;
//    sphere.SetModule(perlin);
//
//    int pos=0;
//    double latFactor = 180.0/h;
//    double lonFactor = 360.0/w;
//    double lat,lon;
//    float min=0,max=0;
//    for (int i = 0; i < h; ++i) {
//        int dh = h/2 - i;
//        for (int j = 0; j <w; ++j) {
//            int dw = j - w/2;
//            lat = dh*latFactor;
//            lon = dw*lonFactor;
//            float s = sphere.GetValue(lat,lon);
//            if(s<min){
//                min=s;
//            }
//            if(s>max){
//                max=s;
//            }
//            double t = (s+1.2293236)/2.5414443;
//
//            ivec3 color;
//            if(s>1.2){
//                //雪山
//                color = vec3(255,255,255);
//            } else if(s<=1.2&&s>1.0){
//                //雪山
//                color = vec3(255,245,245);
//            } else if(s<=1.0&&s>0.7500){
//                //岩石
//                color = vec3(90,77,65);
//            } else if(s<=.7500&&s>.3750){
//                //泥地
//                color = vec3(155,118,83);
//            } else if(s<=.3750&&s>.1250){
//                //草地
//                color = vec3(32,160,0);
//            } else if(s<=.1250&&s>.0625){
//                //沙滩
//                color = vec3(199,158,1);
//            } else if(s<=.0625&&s>.0){
//                color = vec3(0,128,255);
//            } else if(s<=0&&s>-.2500){
//                //浅海
//                color = vec3(0,0,255);
//            } else if(s<=-2.5&&s>-1.0){
//                //深海
//                color = vec3(25,25,112);
//            } else{
//                //深海
//                color = vec3(10,10,52);
//            }
//            data[pos++] = color.r;
//            data[pos++] = color.g;
//            data[pos++] = color.b;
//        }
//    }
//    Logger::Info("{}-{}={}",min,max,min-max);
//    stbi_write_jpg("data/textures/earthmap4k.jpg",w,h,comp,data,100);
}
