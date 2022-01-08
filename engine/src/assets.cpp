//
// Created by koerriva on 2021/12/20.
//

#include "assets.h"
#include "log.h"
#include "sys/stat.h"

namespace wx {
    AssetsLoader* AssetsLoader::_instance = nullptr;

    void AssetsLoader::file_collector(const directory_entry &entry) {
        if(entry.is_directory()){
            WX_CORE_INFO("Dir {}",entry.path().string());
            directory_iterator list(entry.path());
            for (auto& dir:list) {
                file_collector(dir);
            }
        }else{
            ifstream file(entry.path(),ios::ate|ios::binary);
            if(file.is_open()){
                auto filename = entry.path().string();
                auto ext = filename.substr(filename.find_last_of('.')+1);

                streampos size = file.tellg();
                auto& buffer = data[filename];
                if(size>0){
                    buffer.resize(int(size));
                    file.seekg(0,ios::beg);
                    char* p = (char*)(buffer.data());
                    file.read(p,size);
                }
                file.close();
                info[filename].ext = ext;
                info[filename].size = buffer.size();
                struct stat file_state{};
                stat(entry.path().string().c_str(),&file_state);
                info[filename].mtime = file_state.st_mtime;
                info[filename].path = entry.path().string();
//                WX_CORE_INFO("File {},{}",filename,buffer.size());
            }
        }
    }

    void AssetsLoader::init() {
        directory_iterator list(root);
        for (auto& entry:list){
            file_collector(entry);
        }
    }

    const char *AssetsLoader::loadShader(const char *name, int type) {
        string file_ext;
        if(type==1){
            file_ext = ".vert";
        }else if(type==3){
            file_ext = ".frag";
        }else if(type==2){
            file_ext = ".geom";
        }

        string filepath = string(root).append(FILE_SPLITER).append("shader").append(FILE_SPLITER).append(name).append(file_ext);
        path dir(filepath);
        WX_CORE_INFO("Find Shader {}",dir.string());

        if(data.count(filepath)>0){
            auto& buffer = data[filepath];
            if(buffer.size()==info[filepath].size){
                WX_CORE_INFO("Add Terminated Char {}",dir.string());
                buffer.push_back(0);
            }
            return (const char*)(buffer.data());
        }else{
            WX_CORE_ERROR("Can't Find Shader {}",dir.string());
            return nullptr;
        }
    }

    const unsigned char *AssetsLoader::loadTexture(const char *name, int *len) {
        string filepath = string(root).append(FILE_SPLITER).append("textures").append(FILE_SPLITER).append(name);
        path dir(filepath);
        WX_CORE_INFO("Find TextureLoader {}",dir.string());

        if(data.count(filepath)>0){
            *len = data[filepath].size();
            return data[filepath].data();
        }else{
            WX_CORE_ERROR("Can't Find TextureLoader {}",dir.string());
            return nullptr;
        }
    }

    const unsigned char *AssetsLoader::loadRawData(const char *name, int *len) {
        string filepath = string(root).append(FILE_SPLITER).append(name);
        path dir(filepath);
        WX_CORE_INFO("Find Raw {}",dir.string());

        if(data.count(filepath)>0){
            *len = data[filepath].size();
            return data[filepath].data();
        }else{
            WX_CORE_ERROR("Can't Find Raw {}",dir.string());
            return nullptr;
        }
    }
}