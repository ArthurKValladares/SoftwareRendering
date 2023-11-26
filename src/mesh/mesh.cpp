#include "mesh/mesh.h"

void Mesh::SetupTriangles() {
	triangles.reserve(indices.size() / 3);
	for (int i = 0; i < indices.size(); i += 3) {
        const Vertex& v0 = vertices[indices[i]];
        const Vertex& v1 = vertices[indices[i + 1]];
        const Vertex& v2 = vertices[indices[i + 2]];

        const Triangle triangle = Triangle{
            v0,
            v1,
            v2
        };
        
        triangles.push_back(triangle);
    }
    screen_triangles.resize(indices.size() / 3);
}

TriangleTileMap Mesh::SetupScreenTriangles(SDL_Surface *surface, const ScreenTileData& tile_data, const Mat4f32& proj_model) {
    // TODO: This is super inneficient
    TriangleTileMap triangle_map;

    const u32 num_tasks = tile_data.num_tasks();

    std::vector<Rect2D> tile_rects;
    tile_rects.reserve(num_tasks);
    for (int tile_index = 0; tile_index < num_tasks; ++tile_index) {
        tile_rects.push_back(tile_data.tile_for_index(surface, tile_index));
    }

    for (u64 i = 0; i < triangles.size(); ++i) {
        const ScreenTriangle st = project_triangle_to_screen(surface, proj_model, triangles[i]);
        const Rect2D triangle_bb = bounding_box(st.p0, st.p1, st.p2);
        for (u32 tile_index = 0; tile_index < num_tasks; ++tile_index) {
            const Rect2D& tile_rect = tile_rects[tile_index];
            const std::optional<Rect2D> opt_bounding_box = Intersection(tile_rect, triangle_bb);
            if (opt_bounding_box.has_value()) {
                if (!triangle_map.count(tile_index)) {
                    triangle_map.insert({ 
                        tile_index, 
                        TriangleTileValue {
                            tile_rect,
                            {}
                        } 
                     });
                }
                triangle_map[tile_index].values.push_back({ opt_bounding_box.value(),i });
            }
        }
        screen_triangles[i] = st;
    }
    return triangle_map;
}