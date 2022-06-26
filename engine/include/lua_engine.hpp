#pragma once

#include <sol/sol.hpp>
#include <functional>
#include "ecs.h"
#include "components.h"

namespace wx
{
    class ScriptEngine{
        public:
            static void Init(){
                _instance = new ScriptEngine();
                _instance->init();
            }

            static void Load(std::string& key,std::string& code,entity_id entity,Transform* transform){
                _instance->load(key,code,entity,transform);
            }

//            template<class R, class... ARGS>
//            static std::function<R(ARGS...)> Run(std::string code, const char* fn){
//                return _instance->run(code,fn);
//            }

            static void InvokeStartFn(std::string& key){
                _instance->invoke_start(key);
            }

            static void InvokeUpdateFn(std::string& key,float t){
                _instance->invoke_update(key,t);
            }

            static void InvokeExitFn(std::string& key){
                _instance->invoke_exit(key);
            }

            static void Cleanup(){
                delete _instance;
            }

        private:
            static ScriptEngine* _instance;

            sol::state lua;

            ScriptEngine();
            ~ScriptEngine();

            void init();

            void load(std::string& name,std::string& code,entity_id entity,Transform* transform);

            void invoke_start(std::string& key){
                auto self = lua[key];
                self["OnStart"](self);
            }

            void invoke_update(std::string& key,float t){
                auto self = lua[key];
                self["OnUpdate"](self,t);
            }

            void invoke_exit(std::string& key){
                auto self = lua[key];
                self["OnExit"](self);
            }

            void cleanup();
    };
} // namespace wx
