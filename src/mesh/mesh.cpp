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

void Mesh::SetupScreenTriangles(SDL_Surface *surface, const Mat4f32& proj_model) {
    for (int i = 0; i < triangles.size(); ++i) {
        screen_triangles[i] = project_triangle_to_screen(surface, proj_model, triangles[i]);
    }
}