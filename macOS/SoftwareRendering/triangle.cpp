#include "triangle.hpp"
#include "defs.h"
#include "vec4i32.hpp"
#include "edge_function.hpp"

Triangle Triangle::rotated(Point2D pivot, float angle) const {
    return Triangle{
        rotate_point(v0, pivot, angle),
        rotate_point(v1, pivot, angle),
        rotate_point(v2, pivot, angle),
        c0,
        c1,
        c2,
        u0,
        u1,
        u2,
    };
}

Rect2D Triangle::bounding_box() const {
    const int minY = MIN3(v0.y, v1.y, v2.y);
    const int minX = MIN3(v0.x, v1.x, v2.x);
    const int maxX = MAX3(v0.x, v1.x, v2.x);
    const int maxY = MAX3(v0.y, v1.y, v2.y);
    return Rect2D{minX, minY, maxX, maxY};
}
