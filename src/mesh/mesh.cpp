#include "mesh/mesh.h"

TriangleTileMap Mesh::SetupScreenTriangles(const ScreenTileData& tile_data, const Mat4f32& proj_model) const {
    TriangleTileMap triangle_map;
    const u32 num_tasks = tile_data.num_tasks();
    triangle_map.tile_rects.resize(num_tasks);
    triangle_map.values.resize(num_tasks);
    triangle_map.screen_triangles.resize(indices.size() / 3);

    for (int tile_index = 0; tile_index < num_tasks; ++tile_index) {
        triangle_map.tile_rects[tile_index] = tile_data.tile_for_index(tile_index);
    }

    triangle_map.Update(*this, tile_data, proj_model);

    return triangle_map;
}

void Mesh::Free() {
    for (auto &texture : texture_map) {
        texture.free();
    }
}

void TriangleTileMap::Update(const Mesh& mesh, const ScreenTileData& tile_data, const Mat4f32& proj_model) {
    for (auto& value : values) {
        value = {};
    }

    // TODO: Maybe this is worth multi-threading afterall
    for (u64 i = 0; i < mesh.indices.size(); i += 3) {
        const u64 st_i = i / 3;

        const Triangle triangle = Triangle {
            mesh.vertices[mesh.indices[i]],
            mesh.vertices[mesh.indices[i + 1]],
            mesh.vertices[mesh.indices[i + 2]],
            mesh.material_ids[st_i]
        };

        const ScreenTriangle st = project_triangle_to_screen(tile_data.total_width, tile_data.total_height, proj_model, triangle);
        const Rect2D triangle_bb = BoundingBox(st.v0.p, st.v1.p, st.v2.p);
        const IndexBounds index_bounds = tile_data.index_bounds_for_bb(triangle_bb);
        for (u32 row_index = index_bounds.min_row; row_index <= index_bounds.max_row; ++row_index) {
            for (u32 col_index = index_bounds.min_col; col_index <= index_bounds.max_col; ++col_index) {
                const u32 tile_index = row_index * (tile_data.cols) + col_index;
                const Rect2D& tile_rect = tile_rects[tile_index];
                const std::optional<Rect2D> opt_bounding_box = Intersection(tile_rect, triangle_bb);
                if (opt_bounding_box.has_value()) {
                    values[tile_index].push_back({ opt_bounding_box.value(), st_i });
                }
            }
        }
        screen_triangles[st_i] = st;
    }
}