#include "triangle.h"

Point2D hacky_project_to_surface(int width, int height, Point3D_f point) {
    const float sx = (point.x + 1.0) / (2.0) * width;
    const float sy = (point.y + 1.0) / (2.0) * height;
    // NOTE: It's fine to just truncate the float to the pixel (int) value it is
    return Point2D{ (int) sx, (int) sy };
}

ScreenTriangle project_triangle_to_screen(int width, int height, const Mat4f32& proj_model, const Triangle& triangle) {
    const Vec4f32 pv0 = proj_model * Vec4f32(triangle.v0.p, 1.0);
    const Vec4f32 pv1 = proj_model * Vec4f32(triangle.v1.p, 1.0);
    const Vec4f32 pv2 = proj_model * Vec4f32(triangle.v2.p, 1.0);

    // TODO: Cleanup the depth stuff
    const Point2D sv0 = hacky_project_to_surface(width, height, Point3D_f(pv0.x(), pv0.y(), pv0.z()));
    const Point2D sv1 = hacky_project_to_surface(width, height, Point3D_f(pv1.x(), pv1.y(), pv1.z()));
    const Point2D sv2 = hacky_project_to_surface(width, height, Point3D_f(pv2.x(), pv2.y(), pv2.z()));

    return ScreenTriangle{
        ScreenVertex{ sv0, pv0.z(), triangle.v0.uv },
        ScreenVertex{ sv1, pv1.z(), triangle.v1.uv },
        ScreenVertex{ sv2, pv2.z(), triangle.v2.uv },
        triangle.material_id
    };
}