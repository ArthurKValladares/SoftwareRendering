#ifndef triangle_h
#define triangle_h

#include "point.h"
#include "color.h"
#include "uv.h"
#include "rect.h"

// Assumes counter-clockwise winding order
struct Triangle {
    Point2D v0;
    Point2D v1;
    Point2D v2;
    Color c0;
    Color c1;
    Color c2;
    UV u0;
    UV u1;
    UV u2;
    
    Triangle rotated(Point2D pivot, float angle) const;
    Rect2D bounding_box() const;
};

#endif
