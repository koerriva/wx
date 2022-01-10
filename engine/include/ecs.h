//
// Created by koerriva on 2021/12/21.
//

#ifndef WX_ECS_H
#define WX_ECS_H

#include <cstdint>
#include <typeinfo>
#include <vector>
#include <cstdio>
#include <unordered_map>
#include <queue>
#include <string>

#include "log.h"
namespace wx {
#define MAX_ENTITIES 0xFFFF

#define CAPACITY_ENTITIES 0xFFFFFF
#define GENERATION_BIT 24

#define GET_GENERATION(id) ((id & 0xFF000000) >> GENERATION_BIT)
#define GET_INDEX(id) (id & CAPACITY_ENTITIES)
#define CREATE_ID(generation, index) (generation << GENERATION_BIT) | index

// The upper 8 bits is the generation
// The lower 24 bits is the actual index in the array

    typedef uint32_t entity_id;

    struct generational_ptr {
        entity_id id;
        uint8_t generation;
    };

    class ComponentBase {
    protected:
        generational_ptr component_to_entity[MAX_ENTITIES];//被几个entity引用
        generational_ptr entity_to_component[MAX_ENTITIES];//被几个component引用
        uint32_t last_component = 0;

    public:
        virtual void destroy(entity_id entity) = 0;
        virtual void free() = 0;

        entity_id get_entity(uint32_t component);
        bool has_component(entity_id entity);
    };

    template<typename T>
    class Component : public ComponentBase {
    private:
        std::vector<T> components = {{}};

    public:
        T *create(entity_id entity, T component) {
            if (last_component == MAX_ENTITIES - 1) {
                WX_CORE_ERROR("Maximum amount reached, can't add any more components");
                return nullptr;
            }

            ++last_component;

            uint32_t last_index = last_component;
            components.push_back(component);
            component_to_entity[last_index].id = entity;
            component_to_entity[last_index].generation += 1;
            uint32_t new_component_id = CREATE_ID(component_to_entity[last_index].generation, last_component);

            uint32_t entity_index = GET_INDEX(entity);
            uint8_t entity_generation = GET_GENERATION(entity);
            entity_to_component[entity_index].id = new_component_id;
            entity_to_component[entity_index].generation = entity_generation;

            return &components[last_index];
        }

        T *get_component(entity_id entity) {
            uint32_t entity_index = GET_INDEX(entity);
            uint8_t entity_generation = GET_GENERATION(entity);
            generational_ptr component_pointer = entity_to_component[entity_index];
            if (component_pointer.generation != entity_generation) {
                WX_CORE_ERROR("[zel_component class] get component return nullptr | Entity: {} | Type: {}", entity,
                       typeid(T).name());
                return nullptr;
            }
            uint32_t component_index = GET_INDEX(component_pointer.id);
            return &components[component_index];
        }

        void destroy(entity_id entity) override {
            uint32_t entity_to_destroy_index = GET_INDEX(entity);
            generational_ptr component_pointer = entity_to_component[entity_to_destroy_index];
            if (component_pointer.generation != GET_GENERATION(entity_to_destroy_index)) {
                WX_CORE_ERROR("Generation not equal, component already destroyed.");
                return;
            }
            uint32_t component_to_destroy_index = GET_INDEX(component_pointer.id);
            destroy_function(&components[component_to_destroy_index]);

            if (component_to_destroy_index == last_component) {
                component_to_entity[component_to_destroy_index].id = 0;
                component_to_entity[component_to_destroy_index].generation += 1;
                entity_to_component[entity_to_destroy_index].id = 0;
                entity_to_component[entity_to_destroy_index].generation += 1;
                return;
            }

            //Fill in empty spot in array
            //将最后一个移动到删除位置
            generational_ptr entity_pointer_to_swap = component_to_entity[last_component];
            components[component_to_destroy_index] = components[last_component];
            component_to_entity[component_to_destroy_index] = entity_pointer_to_swap;
            component_to_entity[last_component].generation += 1;

            entity_to_component[GET_INDEX(entity_pointer_to_swap.id)].id = component_pointer.id;
            entity_to_component[entity_to_destroy_index].generation += 1;
        }

        void free() override {
            if (destroy_function == nullptr) {
                components.clear();
                return;
            }
            uint32_t components_size = components.size();
            for (size_t i = 1; i < components_size; ++i) {
                destroy_function(&components[i]);
            }
            components.clear();
        }

        void (*destroy_function)(T *);
    };

    class ShareResourceBase {};

    template<typename T>
    class ShareResource : public ShareResourceBase{
    private:
        std::atomic<T> value;
    public:
        explicit ShareResource(T _value):value(_value){};

        T* Get(){
            return value.load();
        }

        T* Set(T newValue){
            value.store(newValue);
            return value.load();
        }
    };

    typedef struct level _level;

    typedef void(*system_t)(_level *level, float delta_time);

    struct level {
        //components
        std::unordered_map<std::string, ComponentBase *> components;
        //systems
        std::unordered_map<std::string, system_t> systems;
        //entities
        std::vector<uint8_t> entities = {0};
        std::queue<uint32_t> empty_entities_spots;
        //share obj
        std::unordered_map<std::string, ShareResourceBase *> resources;
    };

    level *level_create();

    void level_destroy(level * level);

    void level_register_system(level * level, system_t system_update, const char *system_name);

    void level_unregister_system(level * level, const char *system_name);

    void level_register_share_resource(level * level,ShareResourceBase* value);

    template<typename T>
    void level_register_component(level * level) {
        std::string type_name = typeid(T).name();
        if (level->components.find(type_name) != level->components.end()) {
            return;
        }

        auto *new_component_type = new Component<T>();
        ComponentBase *base_component_type = new_component_type;
        level->components.insert({type_name, base_component_type});
    }

    template<typename T>
    void level_register_component_with_destroy(level * level, void(*destroy_function)(T*)) {
        std::string type_name = typeid(T).name();
        if(level->components.find(type_name)!= level->components.end()){
            return;
        }

        auto *new_component_type = new Component<T>();
        new_component_type-> destroy_function = destroy_function;
        ComponentBase *base_component_type = new_component_type;
        level->components.insert({ type_name,base_component_type});
    }

    entity_id create_entity(level * level);

    void destroy_entity(level * level, entity_id entity);

    template<typename T>
    T* level_add_component(level * level, entity_id entity,T component){
        auto type_name = std::string(typeid(T).name());
        auto *component_type = (Component<T> *) (level->components[type_name]);

        return component_type->create(entity, component);
    }

    template<typename T>
    T *level_get_component(level * level, entity_id entity){
        auto type_name = std::string(typeid(T).name());
        auto *component_type = (Component<T> *) (level->components[type_name]);

        return component_type->get_component(entity);
    }

    template<typename... T>
    bool level_has_components(level * level, entity_id entity){
        std::string component_names[] = {"", std::string(typeid(T).name())...};

        uint32_t types_size = sizeof...(T);
        std::vector<ComponentBase *> type_bases;
        for ( size_t i = 1; i<types_size + 1; i++) {
            auto c = level->components[component_names[i]];
            if(c==nullptr){
                return false;
            }
            type_bases.push_back(level->components[component_names[i]]);
        }

        for (size_t i = 0;i<types_size;++i) {
            if (!type_bases[i]->has_component(entity)){
                return false;
            }
        }

        return true;
    }

    template<typename T>
    void level_insert_share_resource(level* level,T value){
        std::string type_name = typeid(T).name();
        if (level->resources.find(type_name) != level->resources.end()) {
            return;
        }

        auto *new_share_resource = new Component<T>();
        ShareResourceBase * shareResourceBase = new_share_resource;
        level->resources.insert({type_name, shareResourceBase});
    }

    template<typename T>
    T* level_get_share_resource(level* level){
        std::string type_name = typeid(T).name();
        return level->resources[type_name];
    }
}
#endif //WX_ECS_H
