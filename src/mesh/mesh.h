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

struct TriangleTileValueInner {
    Rect2D bounding_box;
    u64 index;
};

struct TriangleTileValue {
    Rect2D tile_rect;
    std::vector<TriangleTileValueInner> values;
};

using TriangleTileMap = std::unordered_map<u32, TriangleTileValue>;
using TextureMap = std::unordered_map<std::string, Texture>;

struct Mesh {
    void SetupTriangles();
    TriangleTileMap SetupScreenTriangles(SDL_Surface *surface, const ScreenTileData& tile_data, const Mat4f32& proj_model);

    std::vector<Vertex> vertices;
    std::vector<int> indices;
    std::vector<Triangle> triangles;
    std::vector<ScreenTriangle> screen_triangles;

    TextureMap texture_map;

    Rect3D_f bb;
};
