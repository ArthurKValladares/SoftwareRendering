#include "mesh/mesh.h"

TriangleTileMap Mesh::SetupScreenTriangles(BumpAllocator &bump, const ScreenTileData& tile_data, const Mat4f32& proj_model) const {
    const u32 num_tasks = tile_data.num_tasks();
    const u32 num_tris = indices.size() / 3;

    TriangleTileMap triangle_map;

    triangle_map.tile_rects = bump.AllocateArray<Rect2D>(num_tasks);
    for (int tile_index = 0; tile_index < num_tasks; ++tile_index) {
        triangle_map.tile_rects[tile_index] = tile_data.tile_for_index(tile_index);
    }

    triangle_map.values = bump.AllocateArray<BumpVec<TriangleTileMap::InnerValue>>(num_tasks);
    for (auto i = 0; i < num_tasks; ++i) {
        triangle_map.values[i] = bump.AllocateVec<TriangleTileMap::InnerValue>(num_tris);
    }

    // TODO: Maybe this is worth multi-threading afterall
    triangle_map.screen_triangles = bump.AllocateArray<ScreenTriangle>(num_tris);
    for (u64 i = 0; i < indices.size(); i += 3) {
        const u64 st_i = i / 3;

        const Triangle triangle = Triangle {
            vertices[indices[i]],
            vertices[indices[i + 1]],
            vertices[indices[i + 2]],
            material_ids[st_i]
        };

        const ScreenTriangle st = project_triangle_to_screen(tile_data.total_width, tile_data.total_height, proj_model, triangle);
        const Rect2D triangle_bb = BoundingBox(st.v0.p, st.v1.p, st.v2.p);
        const IndexBounds index_bounds = tile_data.index_bounds_for_bb(triangle_bb);
        for (u32 row_index = index_bounds.min_row; row_index <= index_bounds.max_row; ++row_index) {
            for (u32 col_index = index_bounds.min_col; col_index <= index_bounds.max_col; ++col_index) {
                const u32 tile_index = row_index * (tile_data.cols) + col_index;
                const Rect2D& tile_rect = triangle_map.tile_rects[tile_index];
                const std::optional<Rect2D> opt_bounding_box = Intersection(tile_rect, triangle_bb);
                if (opt_bounding_box.has_value()) {
                    triangle_map.values[tile_index].push_back({ st_i });
                }
            }
        }
        triangle_map.screen_triangles[st_i] = st;
    }

    return triangle_map;
}

void Mesh::Free() {
    for (auto &texture : texture_map) {
        texture.free();
    }
}