#pragma once

#include <vector>

#include "point.h"
#include "triangle.h"
#include "texture.h"
#include "math/vec3f32.h"
#include "math/mat4f32.h"
#include "vertex.h"
#include "screen_tile.h"

#include <unordered_map>

struct Mesh;
struct TriangleTileMap {
    void Update(const Mesh& mesh, const ScreenTileData& tile_data, const Mat4f32& proj_model);

    struct InnerValue {
        Rect2D bounding_box;
        u64 index;
    };
    
    // TODO: These two can be fixed-size arrays
    std::vector<Rect2D> tile_rects;
    std::vector<std::vector<InnerValue>> values;

    std::vector<ScreenTriangle> screen_triangles;
};
using TextureMap = std::vector<Texture>;

struct Mesh {
    void SetupTriangles();
    TriangleTileMap SetupScreenTriangles(const ScreenTileData& tile_data, const Mat4f32& proj_model) const;
    void Free();
    
    std::vector<Vertex> vertices;
    std::vector<int> indices;

    std::vector<Triangle> triangles;

    TextureMap texture_map;

    struct Material {
        int texture_id;
        float diffuse[3];
    };
    std::vector<Material> materials;
    std::vector<int> material_ids;

    Rect3D_f bb;
};
