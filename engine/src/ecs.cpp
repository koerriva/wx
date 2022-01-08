//
// Created by koerriva on 2021/12/21.
//
#include "ecs.h"

namespace wx {
    level* level_create(){
        return new level;
    }

    void level_destroy(level* level){
        auto level_components_iter = level->components.begin();
        while (level_components_iter!=level->components.end()){
            level_components_iter->second->free();
            delete level_components_iter->second;

            ++level_components_iter;
        }
        delete level;
    }

    void level_register_system(level * level, system_t system_update,const char *system_name){
        printf("register_system %s\n",system_name);
        level->systems.insert({system_name,system_update});
    }

    void level_unregister_system(level * level, const char *system_name){
        printf("unregister_system %s\n",system_name);
        level->systems.erase(system_name);
    }


    entity_id ComponentBase::get_entity(uint32_t component) {
        uint32_t component_index = GET_INDEX(component);
        uint8_t component_generation = GET_GENERATION(component);
        generational_ptr entity_pointer = component_to_entity[component_index];
        if(entity_pointer.generation != component_generation){
            return 0;
        }

        return entity_pointer.id;
    }

    bool ComponentBase::has_component(entity_id entity) {
        uint32_t entity_index = GET_INDEX(entity);
        uint8_t  entity_generation = GET_GENERATION(entity);
        generational_ptr component_pointer = entity_to_component[entity_index];
        return component_pointer.generation == entity_generation;
    }
}
