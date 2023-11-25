#ifndef triangle_h
#define triangle_h

#include "point.h"
#include "color.h"
#include "uv.h"
#include "rect.h"
#include "math/vec3f32.h"
#include "math/mat4f32.h"
#include "vertex.h"

struct Triangle {
    const Vertex& v0;
    const Vertex& v1;
    const Vertex& v2;
};

struct ScreenTriangle {
    Point2D p0;
    Point2D p1;
    Point2D p2;
};


Point2D hacky_project_to_surface(SDL_Surface* surface, Point3D_f point);
ScreenTriangle project_triangle_to_screen(SDL_Surface* surface, const Mat4f32& proj_model, const Triangle& triangle);

#endif
