#include "math/mat4f32.h"

Mat4f32 Mat4f32::identity() {
    return Mat4f32(
        Vec4f32(1.0, 0.0, 0.0, 0.0),
        Vec4f32(0.0, 1.0, 0.0, 0.0),
        Vec4f32(0.0, 0.0, 1.0, 0.0),
        Vec4f32(0.0, 0.0, 0.0, 1.0)
    );
}

Mat4f32::Mat4f32(Vec4f32 r0, Vec4f32 r1, Vec4f32 r2, Vec4f32 r3) : 
    r0(r0),
    r1(r1),
    r2(r2),
    r3(r3)
{}

Vec4f32 Mat4f32::operator*(const Vec4f32& rhs) const {
    return Vec4f32(r0.dot(rhs), r1.dot(rhs), r2.dot(rhs), r3.dot(rhs));
}