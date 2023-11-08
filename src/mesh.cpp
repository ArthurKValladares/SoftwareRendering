#include "mesh.h"

#include "defs.h"

Mesh Mesh::rotated(Vec3D_f axis, float angle) const {
    Mesh new_mesh = *this;
    for (Triangle &triangle : new_mesh.triangles) {
        triangle = triangle.rotated(axis, angle);
    }
    return new_mesh;
}
