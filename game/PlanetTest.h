//
// Created by koerriva on 2022/1/17.
//

#ifndef WX_PLANETTEST_H
#define WX_PLANETTEST_H

#include "engine.h"

namespace wx {
    struct Sun{};
    struct TerrainShape{
        int lod = 1;
        int last_lod = 1;
        float radius = 10.f;
        float base_radius = 1.f;
        float last_base_radius = 1.f;
//        1 2 4 8 16
//        2*(1-1) 2*(2-1) 2*(3-1) 2*(4-1) 2*(5-1)
//        4 8 16 32 64

        float color[4] = {1.f,1.f,1.f,1.f};
        size_t octaves = 7;
        size_t las_octaves = 7;

        float frequency = 1.f;
        float last_frequency = 1.f;

        float amplitude = 1.f;
        float last_amplitude = 1.f;

        float lacunarity = 2.f;
        float last_lacunarity = 2.f;

        float persistence = 1.f/lacunarity;
    };
    void test_input_system(level* level,float delta);
    void terrain_update_System(level* level,float delta);
}
#endif //WX_PLANETTEST_H
