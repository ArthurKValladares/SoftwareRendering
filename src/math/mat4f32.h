#pragma once

#include "math/vec4f32.h"

struct Mat4f32 {
    static Mat4f32 identity();

    Mat4f32(Vec4f32 r0, Vec4f32 r1, Vec4f32 r2, Vec4f32 r3);

    Mat4f32 transpose() const;
    
    Vec4f32 operator*(const Vec4f32& rhs) const;
    Mat4f32 operator*(const Mat4f32& rhs) const;

    Vec4f32 r0; 
    Vec4f32 r1;
    Vec4f32 r2;
    Vec4f32 r3;
};
