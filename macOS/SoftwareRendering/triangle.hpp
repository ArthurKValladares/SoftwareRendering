#ifndef triangle_hpp
#define triangle_hpp

#include "point.hpp"
#include "color.hpp"
#include "rect.hpp"

// Assumes counter-clockwise winding order
struct Triangle {
    Point2D a;
    Point2D b;
    Point2D c;
    Color ca;
    Color cb;
    Color cc;
};

Triangle rotate_triangle(Triangle triangle, Point2D pivot, float angle);
Rect2D TriangleBoundingBox(Triangle triangle);

#endif
