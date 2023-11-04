#pragma once

#include "defs.h"
#ifdef __ARM_NEON
#include "sse2neon.h"
#else
#include <emmintrin.h>
#endif

struct Vec4f32 {
    Vec4f32();
    Vec4f32(float val);
    Vec4f32(float a, float b, float c, float d);
    Vec4f32(__m128 a);
    
    void store(float* dest) const;
    
    Vec4f32 operator+(const Vec4f32& rhs) const;
    Vec4f32 operator*(const Vec4f32& rhs) const;
    Vec4f32 operator/(const Vec4f32& rhs) const;
private:
    __m128 _mf;
};
