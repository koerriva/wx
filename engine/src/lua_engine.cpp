#include "lua_engine.hpp"
#include "log.h"

namespace wx {
    ScriptEngine* ScriptEngine::_instance = nullptr;

    ScriptEngine::ScriptEngine(){

    }

    ScriptEngine::~ScriptEngine(){
        WX_CORE_TRACE("Drop Lua Engine");
    }

    void ScriptEngine::init(){
        lua.open_libraries(sol::lib::base,sol::lib::package);
        lua.script("print('脚本系统初始化')");

        lua.script("function f(a,b) return a+b end");
        std::function<int(int,int)> f = lua["f"];
        WX_CORE_TRACE("Test script 1+1={}",f(1,1));
    }

    void ScriptEngine::cleanup(){

    }
}