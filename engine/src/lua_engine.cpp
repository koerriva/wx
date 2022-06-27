#include "lua_engine.hpp"
#include "log.h"
#include "components.h"
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

namespace wx {
    ScriptEngine* ScriptEngine::_instance = nullptr;

    ScriptEngine::ScriptEngine(){

    }

    ScriptEngine::~ScriptEngine(){
        WX_CORE_TRACE("Drop Lua Engine");
    }

    void ScriptEngine::init(){
        lua.open_libraries(sol::lib::base,sol::lib::package,sol::lib::math,sol::lib::string,sol::lib::os,sol::lib::io,sol::lib::bit32);
        lua.script("print('脚本系统初始化')");

        auto entities = lua.require_file("entities","data\\script\\entities.lua");

        //bind components
        lua["glm"].get_or_create<sol::table>().new_usertype<glm::vec3>("vec3",sol::call_constructor,
                                                                       sol::constructors<glm::vec3(const float&),glm::vec3(const float&, const float&, const float&)>(),
                                    "x", &glm::vec3::x,
                                    "y", &glm::vec3::y,
                                    "z", &glm::vec3::z,
                                    "r", &glm::vec3::r,
                                    "g", &glm::vec3::g,
                                    "b", &glm::vec3::b,
                                    "clamp", [](const glm::vec3* vec_a,const glm::vec3* vec_min,const glm::vec3* vec_max) {return clamp(*vec_a,*vec_min,*vec_max);},
                                    "normalize", [](const glm::vec3* vec_a) {return normalize(*vec_a);},
                                    "perlin",[](const glm::vec3* vec_a) {return perlin(*vec_a);},
                                    sol::meta_function::to_string,[] (const glm::vec3* vec) -> std::string {return glm::to_string(*vec);},
                                    sol::meta_function::addition,[] (const glm::vec3* vec_a,const  glm::vec3* vec_b) {return (*vec_a)+(*vec_b);},
                                    sol::meta_function::subtraction,[] (const glm::vec3* vec_a,const  glm::vec3* vec_b) {return (*vec_a)-(*vec_b);},
                                    sol::meta_function::multiplication,[] (const glm::vec3* vec,const float a) {return (*vec)*a;},
                                    sol::meta_function::division,[] (const glm::vec3* vec,const float a) {return (*vec)/a;},
                                    sol::meta_function::unary_minus,[] (const glm::vec3* vec) {return (*vec)*(-1.f);},
                                    sol::meta_function::equal_to,[] (const glm::vec3* vec_a,const  glm::vec3* vec_b) {return (*vec_a)==(*vec_b);}
        );


        lua.new_usertype<Transform>("Transform",sol::call_constructor,sol::constructors<Transform()>(),
                "position",&Transform::position,
                "scale",&Transform::scale,
                "rotate",&Transform::Rotate);

        lua.script("function f(a,b) return a+b end");
        std::function<int(int,int)> f = lua["f"];
        WX_CORE_TRACE("Test script 1+1={}",f(1,1));
    }

    void ScriptEngine::load(std::string& name,std::string& code,entity_id entity,Transform* transform) {
        auto result = lua.script(code);
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "The code has failed to run!\n"
                      << err.what() << "\nPanicking and exiting..."
                      << std::endl;
            throw std::runtime_error(err.what());
        }

        auto self = lua[name];
        self["entity"] = entity;
        self["transform"] = transform;
    }

    void ScriptEngine::cleanup(){

    }


}