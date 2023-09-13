#include "triangle.hpp"
#include "defs.h"

Triangle rotate_triangle(Triangle triangle, Point2D pivot, float angle) {
    return Triangle{rotate_point(triangle.a, pivot, angle),
                    rotate_point(triangle.b, pivot, angle),
                    rotate_point(triangle.c, pivot, angle),
                    triangle.ca,
                    triangle.cb,
                    triangle.cc};
}

Rect2D TriangleBoundingBox(Triangle triangle) {
    const int minY = MIN3(triangle.a.y, triangle.b.y, triangle.c.y);
    const int minX = MIN3(triangle.a.x, triangle.b.x, triangle.c.x);
    const int maxX = MAX3(triangle.a.x, triangle.b.x, triangle.c.x);
    const int maxY = MAX3(triangle.a.y, triangle.b.y, triangle.c.y);
    return Rect2D{minX, minY, maxX, maxY};
}
