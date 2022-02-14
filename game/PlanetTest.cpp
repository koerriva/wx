//
// Created by koerriva on 2022/1/17.
//

#include "PlanetTest.h"
#include "systems.h"

#define MAX_VERTEX_BUFFER (512 * 1024)
#define MAX_ELEMENT_BUFFER (128 * 1024)

namespace wx {
    void planet_game(App* app){
        app->InsertResource(WindowConfig{"Metaverse Engine 0.3.2",1440,900});

        wx::Camera camera{};
        camera.position = glm::vec3(0.0f,2.0f,20.0f);
        app->Spawn(camera,wx::MainCamera{});

        wx::Light sun{};
        sun.type = wx::Light::directional;
        sun.color = vec3{1.0f};
        sun.position = vec3(5.f,5.f,0.f);
        sun.direction = normalize(-sun.position);
        sun.intensity = 20;
        sun.shadow_map = wx::TextureLoader::LoadDepthMap(4096, 4096);
        sun.has_shadow_map = 1;
        sun.near_plane = -20.f;
        sun.far_plane = 40.f;
        sun.p = glm::ortho(-40.0f, 40.0f, -40.0f, 40.0f, sun.near_plane, sun.far_plane);;
        sun.v = glm::lookAt(sun.position, vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        app->Spawn(sun,CastShadow{},Sun{});

        Spatial3d Planet01{.name="Planet01"};
        Transform transform{.scale={1.f,1.f,1.f}};
        TerrainShape terrainShape{.resolution=30,.last_resolution=30};
        terrain = new Terrain(&terrainShape);
        Mesh terrainMesh = terrain->GetMesh();
        app->Spawn(Planet01,transform,terrainMesh,ReceiveShadow{},terrainShape);

//        Skybox skybox{};
//        skybox.cubemap = TextureLoader::LoadRendererCubeMap(1024, 1024);
//        skybox.weather = 0.5;
//        skybox.sun_pos = sun.position;
//        skybox.rot_stars = mat3_cast(quat(vec3(radians(30.f), radians(90.f),0)));
//
//        skybox.tint = TextureLoader::Load("skybox\\tint.png");
//        skybox.tint2 = TextureLoader::Load("skybox\\tint2.png");
//        skybox.sun = TextureLoader::Load("skybox\\sun.png");
//        skybox.moon = TextureLoader::Load("skybox\\moon.png");
//        skybox.clouds1 = TextureLoader::Load("skybox\\clouds1.png");
//        skybox.clouds2 = TextureLoader::Load("skybox\\clouds2.png");
////        Mesh skybox_mesh = Assets::LoadStaticModel("model\\cube.gltf");
//        Mesh skybox_mesh{};
//        skybox_mesh.primitives.push_back(Assets::UnitCube(16));
//        app->Spawn(Spatial3d{.name="Skybox"},Transform{},skybox,skybox_mesh);

        app->AddSystem(SYSTEM_NAME(test_input_system),test_input_system);
        app->AddSystem(SYSTEM_NAME(third_person_camera_controller_system),third_person_camera_controller_system);
        app->AddSystem(SYSTEM_NAME(terrain_update_System),terrain_update_System);
        WX_INFO("_run-------------------------------------");
    }

    void test_input_system(level* level,float delta){
        auto inputState = level_get_share_resource<InputState>(level);

        Light* sun = nullptr;

        auto entities_iter = level->entities.begin();
        auto entities_begin = level->entities.begin();

        while (entities_iter != level->entities.end()){
            uint32_t entity_idx = entities_iter-entities_begin;
            entity_id entity = CREATE_ID((*entities_iter),entity_idx);
            if(entity!=0 && level_has_components<Sun>(level,entity)){
                sun = level_get_component<Light>(level,entity);
            }
            entities_iter++;
        }

        if(inputState->GetKeyPressed(InputState::K)){
            WX_INFO("Pressed K");
            if(sun){
                sun->state = sun->state==0?1:0;
            }
        }

        if(inputState->GetKeyPressed(InputState::Num1)){
            WX_INFO("Pressed Num1");
            if(level_has_share_resource<RenderState>(level)){
                auto render_state = level_get_share_resource<RenderState>(level);
                if(render_state->mode==RenderState::Shader){
                    render_state->mode=RenderState::Wireframe;
                }else{
                    render_state->mode=RenderState::Shader;
                }
            }
        }
    }

    void terrain_update_System(level* level,float delta){
        ::entity_id terrain_entity = 0;
        auto entities_iter = level->entities.begin();
        auto entities_begin = level->entities.begin();

        while (entities_iter != level->entities.end()){
            uint32_t entity_idx = entities_iter-entities_begin;
            entity_id entity = CREATE_ID((*entities_iter),entity_idx);
            if(entity!=0 && level_has_components<TerrainShape>(level,entity)){
                terrain_entity = entity;
                break;
            }
            entities_iter++;
        }

        auto terrainShape = level_get_component<TerrainShape>(level,terrain_entity);
        auto terrainMesh = level_get_component<Mesh>(level,terrain_entity);
        auto terrainTransform = level_get_component<Transform>(level,terrain_entity);

        for (auto& primitive:terrainMesh->primitives) {
            primitive.material.albedo_factor = make_vec4(terrainShape->color);
        }

//        terrainTransform->scale = vec3{terrainShape->radius};

        if(terrainShape->last_resolution!=terrainShape->resolution){
            WX_INFO("update resolution : {}",terrainShape->resolution);
            terrainShape->last_resolution = terrainShape->resolution;
            terrain->Update(terrainShape,terrainMesh);
        }

        if(terrainShape->last_radius!=terrainShape->radius){
            WX_INFO("update radius : {}",terrainShape->radius);
            terrainShape->last_radius = terrainShape->radius;
            terrain->Update(terrainShape,terrainMesh);
        }
    }

    void nuklear_update_system(level* level,float delta){
        ::entity_id terrain_entity = 0;
        auto entities_iter = level->entities.begin();
        auto entities_begin = level->entities.begin();

        while (entities_iter != level->entities.end()){
            uint32_t entity_idx = entities_iter-entities_begin;
            entity_id entity = CREATE_ID((*entities_iter),entity_idx);
            if(entity!=0 && level_has_components<TerrainShape>(level,entity)){
                terrain_entity = entity;
                break;
            }
            entities_iter++;
        }
        if(terrain_entity==0)return;

        auto terrainShape = level_get_component<TerrainShape>(level,terrain_entity);

        if(!level_has_share_resource<NuklearContext>(level))return;
        auto nkcontext = level_get_share_resource<NuklearContext>(level);
        auto glfw = (struct nk_glfw*)nkcontext->glfw;
        auto ctx = (struct nk_context*)nkcontext->ctx;

        nk_glfw3_new_frame(glfw);
        /* GUI */
        if (nk_begin(ctx, "Demo", nk_rect(10, 10, 230, 300),
                     NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
                     NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
        {
            nk_layout_row_dynamic(ctx, 25, 1);
            nk_property_int(ctx, u8"faces:", 2, &terrainShape->resolution, 120, 1, 1);
            nk_layout_row_dynamic(ctx, 25, 1);
            nk_property_float(ctx, u8"radius:", 1, &terrainShape->radius, 100, 0.1, 0.1);
            nk_layout_row_dynamic(ctx, 25, 1);

            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, u8"颜色:", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 25, 1);
            nk_colorf shapeColor{terrainShape->color[0],terrainShape->color[1],terrainShape->color[2],terrainShape->color[3]};
            if (nk_combo_begin_color(ctx, nk_rgba_cf(shapeColor), nk_vec2(nk_widget_width(ctx),400))) {
                nk_layout_row_dynamic(ctx, 120, 1);
                shapeColor = nk_color_picker(ctx, shapeColor, NK_RGBA);
                nk_layout_row_dynamic(ctx, 25, 1);
                shapeColor.r = nk_propertyf(ctx, "#R:", 0, shapeColor.r, 1.0f, 0.01f,0.005f);
                shapeColor.g = nk_propertyf(ctx, "#G:", 0, shapeColor.g, 1.0f, 0.01f,0.005f);
                shapeColor.b = nk_propertyf(ctx, "#B:", 0, shapeColor.b, 1.0f, 0.01f,0.005f);
                shapeColor.a = nk_propertyf(ctx, "#A:", 0, shapeColor.a, 1.0f, 0.01f,0.005f);

                terrainShape->color[0] = shapeColor.r;
                terrainShape->color[1] = shapeColor.g;
                terrainShape->color[2] = shapeColor.b;
                terrainShape->color[3] = shapeColor.a;
                nk_combo_end(ctx);
            }
        }
        nk_end(ctx);
        nk_glfw3_render(glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
    }
}

int main(int argc,char** argv) {

    system("chcp 65001");

    wx::Log::Init();
    wx::AssetsLoader::Init();

    WX_INFO("我的游戏引擎 0.2");
    auto* app = new wx::App();
    app->AddPlugin(SYSTEM_NAME(wx::gltf_viewer_game),wx::planet_game);
    app->Run();
}