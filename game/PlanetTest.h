//
// Created by koerriva on 2022/1/17.
//

#ifndef WX_PLANETTEST_H
#define WX_PLANETTEST_H

#include "engine.h"

namespace wx {
    struct Sun{};

    struct TerrainShape{
        int resolution = 1;
        int last_resolution = 1;
        float radius = 10.f;
        float last_radius = 10.f;
        float base_radius = 1.f;
        float amplitude = 1.f;
        float color[4] = {1.f,1.f,1.f,1.f};

        [[nodiscard]] vec3 point_on_planet(vec3 point) const{
            return point * radius;
        };
    };


    class Terrain {
    public:
        explicit Terrain(TerrainShape* terrainShape){
            m_TerrainMesh.name = "Planet01";
            generateMesh(terrainShape);
        }

        void Update(TerrainShape* terrainShape,Mesh* terrainMesh){
            generateMesh(terrainShape);
            terrainMesh->primitives.clear();
            for (auto& priv:m_TerrainMesh.primitives) {
                terrainMesh->primitives.push_back(priv);
            }
        }

        Mesh& GetMesh(){
            return m_TerrainMesh;
        }
    private:
        Mesh m_TerrainMesh;

        void generateMesh(TerrainShape* terrainShape){
            auto frontFace = GetUnitSphereFace(vec3(0,0,1),terrainShape);
            auto backFace = GetUnitSphereFace(vec3(0,0,-1),terrainShape);
            auto upFace = GetUnitSphereFace(vec3(0,1,0),terrainShape);
            auto downFace = GetUnitSphereFace(vec3(0,-1,0),terrainShape);
            auto leftFace = GetUnitSphereFace(vec3(-1,0,0),terrainShape);
            auto rightFace = GetUnitSphereFace(vec3(1,0,0),terrainShape);

            m_TerrainMesh.primitives.clear();
            m_TerrainMesh.primitives.push_back(frontFace);
            m_TerrainMesh.primitives.push_back(backFace);
            m_TerrainMesh.primitives.push_back(upFace);
            m_TerrainMesh.primitives.push_back(downFace);
            m_TerrainMesh.primitives.push_back(leftFace);
            m_TerrainMesh.primitives.push_back(rightFace);
        }

        static Mesh::primitive_t GetUnitSphereFace(vec3 normal, TerrainShape* shape) {
            int resolution = shape->resolution;

            int num_vertices = resolution*resolution;
            int num_indices = (resolution-1)*(resolution-1)*6;

            SurfaceData surfaceData;
            auto& vertices = surfaceData.vertices;
            auto& normals = surfaceData.normals;
            auto& texcoords = surfaceData.texCoords;
            auto& indices = surfaceData.indices;

            vertices.resize(num_vertices,vec3(0));
            normals.resize(num_vertices,vec3(0));
            texcoords.resize(num_vertices,vec2(0));
            indices.resize(num_indices,0);

            vec3 axisA = vec3{normal.y,normal.z,normal.x};
            vec3 axisB = cross(normal,axisA);

            int tri_index = 0;
            for (int y = 0; y < resolution; ++y) {
                for (int x = 0; x < resolution; ++x) {
                    int i = x+y*resolution;
                    vec2 percent = vec2(x,y)/(float(resolution)-1.f);
                    vec3 pointOnUnitCube = normal+(percent.x-0.5f)*2.f*axisA+(percent.y-0.5f)*2.f*axisB;
                    vec3 pointOnUnitSphere = normalize(pointOnUnitCube);

                    vertices[i] = shape->point_on_planet(pointOnUnitSphere);

                    texcoords[i] = vec2(0.f);

                    if(x!=resolution-1&&y!=resolution-1){
                        indices[tri_index+2] = i;
                        indices[tri_index+1] = i+resolution+1;
                        indices[tri_index] = i+resolution;

                        indices[tri_index+5] = i;
                        indices[tri_index+4] = i+1;
                        indices[tri_index+3] = i+resolution+1;
                        tri_index +=6;
                    }
                }
            }

            for (int i = 0; i < num_indices;) {
                vec3 a = vertices[indices[i]];
                vec3 b = vertices[indices[i+1]];
                vec3 c = vertices[indices[i+2]];

                vec3 ab = b-a;
                vec3 bc = c-b;
                vec3 ca = a-c;

                vec3 cross_ab_bc = cross(ab,bc) * -1.0f;
                vec3 cross_bc_ca = cross(bc,ca) * -1.0f;
                vec3 cross_ca_ab = cross(ca,ab) * -1.0f;

                normals[indices[i]] += cross_ab_bc+cross_bc_ca+cross_ca_ab;
                normals[indices[i+1]] += cross_ab_bc+cross_bc_ca+cross_ca_ab;
                normals[indices[i+2]] += cross_ab_bc+cross_bc_ca+cross_ca_ab;

                i+=3;
            }

            for (int i = 0; i < num_vertices; ++i) {
                normals[i] = normalize(normals[i]);
            }

            Mesh::primitive_t surface = Assets::GenMesh(surfaceData);
            return surface;
        }
    };

    void test_input_system(level* level,float delta);
    void terrain_update_System(level* level,float delta);

    Terrain* terrain;
}
#endif //WX_PLANETTEST_H
