#pragma once

#include <vector>

#include "point.h"
#include "triangle.h"
#include "texture.h"
#include "math/vector.h"
#include "vertex.h"

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    // TODO: Probably shouldn't really be a part of Mesh struct
    Texture texture;
};
