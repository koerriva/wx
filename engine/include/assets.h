//
// Created by koerriva on 2021/12/20.
//

#ifndef WX_ASSETS_H
#define WX_ASSETS_H

#define FILE_SPLITER "\\"

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
        time_t mtime;
        std::string path;
    };
    class AssetsLoader{
    private:
        const char* root = "data";
        std::unordered_map<std::string,FileInfo> info;
        std::unordered_map<std::string,std::vector<unsigned char>> data;
        void file_collector(const directory_entry& entry);
        AssetsLoader()= default;;
        static AssetsLoader* _instance;

        void init();
        const char* loadShader(const char* name,int type);
        const unsigned char* loadTexture(const char* name,int* len);
        const unsigned char* loadRawData(const char* name,int* len);
        const char* loadText(const char* name,int* len);
        const FileInfo& fileInfo(const char* name);
        void cleanup(){
            data.clear();
        }
    public:
        ~AssetsLoader();

        static void Init(){
            _instance = new AssetsLoader();
            _instance->init();
        }

        static const char* LoadShader(const char* name,int type){
            return _instance->loadShader(name,type);
        }

        static const unsigned char* LoadTexture(const char* name,int* len){
            return _instance->loadTexture(name,len);
        }

        static const unsigned char* LoadRawData(const char* name,int* len){
            return _instance->loadRawData(name,len);
        }

        static const char* LoadText(const char* name,int* len){
            return _instance->loadText(name,len);
        }

        static const FileInfo& FileInfo(const char* name){
            return _instance->fileInfo(name);
        }

        static void Cleanup(){
            _instance->cleanup();
        }
    };
}
#endif //WX_ASSETS_H
