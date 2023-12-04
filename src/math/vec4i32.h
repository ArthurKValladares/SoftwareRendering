#pragma once

#include "defs.h"
#ifdef __ARM_NEON
#include "sse2neon.h"
#else
#include <emmintrin.h>
#endif

struct Vec4f32;
struct Vec4i32 {
    Vec4i32();
    Vec4i32(i32 a, i32 b, i32 c, i32 d);
    Vec4i32(i32 a);
    Vec4i32(__m128i a);
    
    Vec4f32 to_float() const;
    
    i32 x() const;
	i32 y() const;
	i32 z() const;
	i32 w() const;

    bool any_gte(i32 val) const;
    
    Vec4i32 operator*(const i32& rhs) const;

    Vec4i32 operator+(const Vec4i32& rhs) const;
    Vec4i32 operator-(const Vec4i32& rhs) const;
    Vec4i32 operator*(const Vec4i32& rhs) const;
    
    void operator+=(const Vec4i32& rhs);
    void operator-=(const Vec4i32& rhs);
    void operator*=(const Vec4i32& rhs);
    
    Vec4i32 operator|(const Vec4i32& rhs) const;
    Vec4i32 operator^(const Vec4i32& rhs) const;
    Vec4i32 operator!() const;
    
    bool operator==(const Vec4i32& rhs) const;
    bool operator>(const Vec4i32& rhs) const;
    bool operator<(const Vec4i32& rhs) const;
    bool operator>=(const Vec4i32& rhs) const;
    bool operator<=(const Vec4i32& rhs) const;
private:
    union {
        __m128i _mi;
        i32 _xyzw[4];
    };
};
