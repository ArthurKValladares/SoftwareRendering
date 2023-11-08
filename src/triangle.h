#ifndef triangle_h
#define triangle_h

#include "point.h"
#include "color.h"
#include "uv.h"
#include "rect.h"
#include "vector.h"
#include "viewport.h"

struct ScreenTriangle {
    Point2D v0;
    Point2D v1;
    Point2D v2;

    Rect2D bounding_box() const;
};

// Assumes counter-clockwise winding order
// TODO: Will just hold 3 vertices later. Or not exist at all?
struct Triangle {
    Point3D_f v0;
    Point3D_f v1;
    Point3D_f v2;
    UV u0;
    UV u1;
    UV u2;

    Rect2D bounding_box() const;
    ScreenTriangle project_to_surface(SDL_Surface *surface, const Viewport& viewport) const;
};

#endif
