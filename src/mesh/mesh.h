#pragma once

#include <vector>

#include "point.h"
#include "triangle.h"
#include "texture.h"
#include "math/vec3f32.h"
#include "vertex.h"

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<int> indices;
};
