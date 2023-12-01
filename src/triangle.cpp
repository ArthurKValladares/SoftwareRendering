#include "triangle.h"

Point2D hacky_project_to_surface(SDL_Surface* surface, Point3D_f point) {
    const u32 w = surface->w;
    const u32 h = surface->h;
    const float sx = (point.x + 1.0) / (2.0) * w;
    const float sy = (point.y + 1.0) / (2.0) * h;
    return Point2D{ (int)round(sx), (int)round(sy) };
}

ScreenTriangle project_triangle_to_screen(SDL_Surface* surface, const Mat4f32& proj_model, const Triangle& triangle) {
    const Vec4f32 pv0 = proj_model * Vec4f32(triangle.v0.p, 1.0);
    const Vec4f32 pv1 = proj_model * Vec4f32(triangle.v1.p, 1.0);
    const Vec4f32 pv2 = proj_model * Vec4f32(triangle.v2.p, 1.0);

    // TODO: Cleanup
    const Point2D sv0 = hacky_project_to_surface(surface, Point3D_f(pv0.x(), pv0.y(), pv0.z()));
    const Point2D sv1 = hacky_project_to_surface(surface, Point3D_f(pv1.x(), pv1.y(), pv1.z()));
    const Point2D sv2 = hacky_project_to_surface(surface, Point3D_f(pv2.x(), pv2.y(), pv2.z()));

    return ScreenTriangle{
        ScreenVertex{ sv0, triangle.v0.uv },
        ScreenVertex{ sv1, triangle.v1.uv },
        ScreenVertex{ sv2, triangle.v2.uv }
    };
}