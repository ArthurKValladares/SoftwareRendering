#ifndef triangle_h
#define triangle_h

#include "point.h"
#include "color.h"
#include "uv.h"
#include "rect.h"
#include "vector.h"
#include "viewport.h"
#include "vertex.h"

struct ScreenTriangle {
    Point2D v0;
    Point2D v1;
    Point2D v2;

    Rect2D bounding_box() const;
};

struct Triangle {
    const Vertex& v0;
    const Vertex& v1;
    const Vertex& v2;

    ScreenTriangle project_to_surface(SDL_Surface *surface, const Viewport& viewport) const;
};

#endif
