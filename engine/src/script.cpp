#include "systems.h"
#include "lua_engine.hpp"
#include "assets.h"

namespace wx {
    void script_setup_system(level* level, float delta){
        auto entities_iter = level->entities.begin();
        auto entities_begin = level->entities.begin();
        while(entities_iter!= level->entities.end()){
            uint32_t entity_idx = entities_iter-entities_begin;
            entity_id entity = CREATE_ID((*entities_iter),entity_idx);
            if(entity!=0&&level_has_components<LuaScript>(level,entity)){
                LuaScript* luaScript = level_get_component<LuaScript>(level,entity);
                if(!luaScript->cached){
                    int len = 0;
                    std::string code = AssetsLoader::LoadText(luaScript->uri.c_str(),&len);
                    luaScript->cached = true;
                    luaScript->code = code;
                    std::function<void(void)> OnStart = ScriptEngine::Run<void>(luaScript->code,"OnStart");
                    OnStart();
                }
            }
            entities_iter++;
        }
    };
    void script_update_system(level* level, float delta){
        auto entities_iter = level->entities.begin();
        auto entities_begin = level->entities.begin();
        while(entities_iter!= level->entities.end()){
            uint32_t entity_idx = entities_iter-entities_begin;
            entity_id entity = CREATE_ID((*entities_iter),entity_idx);
            if(entity!=0&&level_has_components<LuaScript>(level,entity)){
                LuaScript* luaScript = level_get_component<LuaScript>(level,entity);
                if(luaScript->cached){
                    std::function<void(float)> OnUpdate = ScriptEngine::Run<void,float>(luaScript->code,"OnUpdate");
                    OnUpdate(delta);
                }
            }
            entities_iter++;
        }
    };
}