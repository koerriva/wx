//
// Created by koerriva on 2021/12/20.
//

#ifndef WX_ASSETS_H
#define WX_ASSETS_H

#define FILE_SPLITER "\\"
#ifdef __APPLE__
#define FILE_SPLITER "/"
#endif

#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <iostream>

namespace wx {
    using namespace std;
    using namespace std::filesystem;

    struct FileInfo {
        long long size=0;
        std::string ext;
    };
    class ResourceLoader{
    private:
        const char* root = "data";
        std::unordered_map<std::string,FileInfo> info;
        std::unordered_map<std::string,std::vector<unsigned char>> data;
        void file_collector(const directory_entry& entry);
        ResourceLoader()= default;;
        static ResourceLoader* _instance;

        void init();
        const char* loadShader(const char* name,int type);
        const unsigned char* loadTexture(const char* name,int* len);
        void cleanup(){
            data.clear();
        }
    public:
        ~ResourceLoader(){
            std::cout << "Drop ResourceLoader" << std::endl;
        };

        static void Init(){
            _instance = new ResourceLoader();
            _instance->init();
        }

        static const char* LoadShader(const char* name,int type){
            return _instance->loadShader(name,type);
        }

        static const unsigned char* LoadTexture(const char* name,int* len){
            return _instance->loadTexture(name,len);
        }

        static void Cleanup(){
            _instance->cleanup();
        }
    };
}
#endif //WX_ASSETS_H
