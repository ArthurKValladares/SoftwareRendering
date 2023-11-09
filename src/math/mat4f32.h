#pragma once

#include "math/vec4f32.h"

struct Mat4f32 {
    static Mat4f32 identity();

    Mat4f32(Vec4f32 r0, Vec4f32 r1, Vec4f32 r2, Vec4f32 r3);

    Vec4f32 r0; 
    Vec4f32 r1;
    Vec4f32 r2;
    Vec4f32 r3;
};
