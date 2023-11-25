#pragma once

#include <vector>

#include "point.h"
#include "triangle.h"
#include "texture.h"
#include "math/vec3f32.h"
#include "math/mat4f32.h"
#include "vertex.h"

struct Mesh {
    void SetupTriangles();
    void SetupScreenTriangles(SDL_Surface *surface, const Mat4f32& proj_model);

    std::vector<Vertex> vertices;
    std::vector<int> indices;
    std::vector<Triangle> triangles;
    std::vector<ScreenTriangle> screen_triangles;
};
