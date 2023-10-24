#pragma once

#include "defs.h"
#include "sse2neon.h"

struct Vec4f32 {
    Vec4f32();
    Vec4f32(float val);
    Vec4f32(__m128 a);
    
    void store(float* dest) const;
    
    Vec4f32 operator+(const Vec4f32& rhs) const;
    Vec4f32 operator*(const Vec4f32& rhs) const;
    Vec4f32 operator/(const Vec4f32& rhs) const;
private:
    __m128 _mf;
};
