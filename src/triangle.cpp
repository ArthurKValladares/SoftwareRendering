#include "triangle.h"
#include "defs.h"
#include "SIMD/vec4i32.h"
#include "edge_function.h"

Triangle Triangle::rotated(Vec3D_f axis, float angle) const {
    // TODO: Actually implement rotation function
    return *this;
}

Rect2D Triangle::bounding_box() const {
    const int minY = MIN3(v0.y, v1.y, v2.y);
    const int minX = MIN3(v0.x, v1.x, v2.x);
    const int maxX = MAX3(v0.x, v1.x, v2.x);
    const int maxY = MAX3(v0.y, v1.y, v2.y);
    return Rect2D{minX, minY, maxX, maxY};
}
