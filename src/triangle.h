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
    int material_id;
};

// TODO: Not sure I like This ScreenVertex/ScreenTriangle stuff
struct ScreenVertex {
    Point2D p;
    float depth;
    UV uv;
};

struct ScreenTriangle {
    ScreenVertex v0;
    ScreenVertex v1;
    ScreenVertex v2;
    int material_id;
};


Point2D project_to_surface(int width, int height, Point3D_f point);
ScreenTriangle project_triangle_to_screen(int width, int height, const Mat4f32& proj_model, const Triangle& triangle);

#endif
