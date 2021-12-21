//
// Created by koerriva on 2021/12/21.
//
#include "ecs.h"

namespace wx {
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
