#ifndef integer_hpp
#define integer_hpp

#include "defs.h"
#include "sse2neon.h"

struct SIMDVec4 {
    SIMDVec4(i32 a, i32 b, i32 c, i32 d);
    SIMDVec4(__m128i a);
    
    SIMDVec4 operator+(const SIMDVec4& rhs);
    SIMDVec4 operator-(const SIMDVec4& rhs);
    SIMDVec4 operator*(const SIMDVec4& rhs);
private:
    __m128i _mi;
};

#endif
