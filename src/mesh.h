#pragma once

#include <vector>

#include "point.h"
#include "triangle.h"
#include "texture.h"

struct Mesh {
    std::vector<Triangle> triangles;
    // TODO: Probably shouldn't really be a part of Mesh struct
    Texture texture;
    
    Mesh rotated(Point2D pivot, float angle) const;
};
