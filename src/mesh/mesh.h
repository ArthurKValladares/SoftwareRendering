#pragma once

#include <vector>

#include "point.h"
#include "triangle.h"
#include "texture.h"
#include "math/vec3f32.h"
#include "math/mat4f32.h"
#include "vertex.h"
#include "screen_tile.h"
#include "bump.h"

#include <unordered_map>

struct TriangleTileMap {
    struct InnerValue {
        u64 index;
    };
    
    BumpArray<Rect2D> tile_rects;
    BumpArray<BumpVec<InnerValue>> values;

    BumpArray<ScreenTriangle> screen_triangles;
};

using TextureMap = std::vector<Texture>;
struct Mesh {
    size_t RequiredMemory(const ScreenTileData& tile_data) const;
    TriangleTileMap SetupScreenTriangles(BumpAllocator &bump, const ScreenTileData& tile_data, const Mat4f32& proj_model) const;
    void Free();
    
    std::vector<Vertex> vertices;
    std::vector<int> indices;

    TextureMap texture_map;

    struct Material {
        int texture_id;
        float diffuse[3];
    };
    std::vector<Material> materials;
    std::vector<int> material_ids;

    Rect3D_f bb;
};
