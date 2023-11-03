#include "mesh.hpp"

#include "defs.h"

Mesh Mesh::rotated(Point2D pivot, float angle) const {
    Mesh new_mesh = *this;
    for (Triangle &triangle : new_mesh.triangles) {
        triangle = triangle.rotated(pivot, angle);
    }
    return new_mesh;
}
