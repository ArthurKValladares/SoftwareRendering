#ifndef triangle_hpp
#define triangle_hpp

#include "point.hpp"
#include "color.hpp"
#include "rect.hpp"

// Assumes counter-clockwise winding order
struct Triangle {
    Point2D v0;
    Point2D v1;
    Point2D v2;
    Color c0;
    Color c1;
    Color c2;
};

Triangle rotate_triangle(Triangle triangle, Point2D pivot, float angle);
Rect2D TriangleBoundingBox(Triangle triangle);

#endif
