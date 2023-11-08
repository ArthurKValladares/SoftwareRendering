#ifndef triangle_h
#define triangle_h

#include "point.h"
#include "color.h"
#include "uv.h"
#include "rect.h"
#include "vector.h"

// Assumes counter-clockwise winding order
struct Triangle {
    Point3D_f v0;
    Point3D_f v1;
    Point3D_f v2;
    Color c0;
    Color c1;
    Color c2;
    UV u0;
    UV u1;
    UV u2;
    
    Triangle rotated(Vec3D_f axis, float angle) const;
    Rect2D bounding_box() const;
};

#endif
