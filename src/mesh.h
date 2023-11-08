#pragma once

#include <vector>

#include "point.h"
#include "triangle.h"
#include "texture.h"
#include "vector.h"

struct Vertex {
    Point3D_f p;
    UV uv;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    // TODO: Probably shouldn't really be a part of Mesh struct
    Texture texture;
    
    // TODO: The cloning here is bad, should have a non-const version that modifies it
    Mesh rotated(Vec3D_f axis, float angle) const;
};
