#include "edge_function.hpp"

Vec4i32 EdgeFunction::Init(Point2D v0, Point2D v1, Point2D p) {
        const int A = v0.y - v1.y;
        const int B = v1.x - v0.x;
        const int C = v0.x * v1.y - v0.y * v1.x;

        step_size_x = Vec4i32(A * EdgeFunction::step_increment_x);
        step_size_y = Vec4i32(B * EdgeFunction::step_increment_y);

        Vec4i32 x = Vec4i32(p.x) + Vec4i32(0, 1, 2, 3);
        Vec4i32 y = Vec4i32(p.y);

        return Vec4i32(A) * x + Vec4i32(B) * y + Vec4i32(C);
}
