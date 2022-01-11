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

        auto level_resource_iter = level->resources.begin();
        while (level_resource_iter!=level->resources.end()){
            level_resource_iter->second->free();

            ++level_resource_iter;
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

    entity_id create_entity(level* level)
    {
        if (!level->empty_entities_spots.empty())
        {
            uint32_t empty_spot = level->empty_entities_spots.front();
            level->empty_entities_spots.pop();
            level->entities[empty_spot] += 1;
            entity_id new_entity = CREATE_ID(level->entities[empty_spot], empty_spot);
            return new_entity;
        }

        level->entities.push_back(1);
        return CREATE_ID(1, level->entities.size() - 1);
    }

    void destroy_entity(level* level, entity_id entity)
    {
        uint32_t entity_index = GET_INDEX(entity);
        level->entities[entity_index] += 1;
        level->empty_entities_spots.push(entity_index);

        auto level_components_iterator = level->components.begin();
        while (level_components_iterator != level->components.end())
        {
            level_components_iterator->second->destroy(entity);

            ++level_components_iterator;
        }
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
