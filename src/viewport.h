#pragma once

#include "defs.h"
#include "point.h"

struct Viewport {
    u32 width;
    u32 height;
    float distance;

    Point2D_f viewport_to_surface(SDL_Surface *surface, Point2D_f p) const;
    Point2D_f project_to_viewport(Point3D_f point) const;
    Point2D_f project_to_surface(SDL_Surface *surface, Point3D_f point) const;
};