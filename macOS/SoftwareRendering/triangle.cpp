#include "triangle.hpp"
#include "defs.h"

Triangle rotate_triangle(Triangle triangle, Point2D pivot, float angle) {
    return Triangle{
        rotate_point(triangle.v0, pivot, angle),
        rotate_point(triangle.v1, pivot, angle),
        rotate_point(triangle.v2, pivot, angle),
        triangle.c0,
        triangle.c1,
        triangle.c2
    };
}

Rect2D TriangleBoundingBox(Triangle triangle) {
    const int minY = MIN3(triangle.v0.y, triangle.v1.y, triangle.v2.y);
    const int minX = MIN3(triangle.v0.x, triangle.v1.x, triangle.v2.x);
    const int maxX = MAX3(triangle.v0.x, triangle.v1.x, triangle.v2.x);
    const int maxY = MAX3(triangle.v0.y, triangle.v1.y, triangle.v2.y);
    return Rect2D{minX, minY, maxX, maxY};
}
