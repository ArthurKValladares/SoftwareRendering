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

Mat4f32 Mat4f32::transpose() const {
    return Mat4f32(
        Vec4f32(this->r0.x(), this->r1.x(), this->r2.x(), this->r3.x()),
        Vec4f32(this->r0.y(), this->r1.y(), this->r2.y(), this->r3.y()),
        Vec4f32(this->r0.z(), this->r1.z(), this->r2.z(), this->r3.z()),
        Vec4f32(this->r0.w(), this->r1.w(), this->r2.w(), this->r3.w())
    );
}

Vec4f32 Mat4f32::operator*(const Vec4f32& rhs) const {
    return Vec4f32(r0.dot(rhs), r1.dot(rhs), r2.dot(rhs), r3.dot(rhs));
}

Mat4f32 Mat4f32::operator*(const Mat4f32& rhs) const {
    const Mat4f32 rhs_t = rhs.transpose();

    const float r0_c0 = this->r0.dot(rhs.r0);
    const float r0_c1 = this->r0.dot(rhs.r1);
    const float r0_c2 = this->r0.dot(rhs.r2);
    const float r0_c3 = this->r0.dot(rhs.r3);

    const float r1_c0 = this->r1.dot(rhs.r0);
    const float r1_c1 = this->r1.dot(rhs.r1);
    const float r1_c2 = this->r1.dot(rhs.r2);
    const float r1_c3 = this->r1.dot(rhs.r3);

    const float r2_c0 = this->r2.dot(rhs.r0);
    const float r2_c1 = this->r2.dot(rhs.r1);
    const float r2_c2 = this->r2.dot(rhs.r2);
    const float r2_c3 = this->r2.dot(rhs.r3);

    const float r3_c0 = this->r3.dot(rhs.r0);
    const float r3_c1 = this->r3.dot(rhs.r1);
    const float r3_c2 = this->r3.dot(rhs.r2);
    const float r3_c3 = this->r3.dot(rhs.r3);

    return Mat4f32(
        Vec4f32(r0_c0, r0_c1, r0_c2, r0_c3),
        Vec4f32(r1_c0, r1_c1, r1_c2, r1_c3),
        Vec4f32(r2_c0, r2_c1, r2_c2, r2_c3),
        Vec4f32(r3_c0, r3_c1, r3_c2, r3_c3)
    );
}

// r0x r0y r0z r0w
// r1x r1y r1z r1w
// r2x r2y r2z r2w
// r3x r3y r3z r3w