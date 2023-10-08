#ifndef integer_hpp
#define integer_hpp

#include "defs.h"
#include "sse2neon.h"

struct Vec4i32 {
    Vec4i32(i32 a, i32 b, i32 c, i32 d);
    Vec4i32(__m128i a);
    
    Vec4i32 operator+(const Vec4i32& rhs) const;
    Vec4i32 operator-(const Vec4i32& rhs) const;
    Vec4i32 operator*(const Vec4i32& rhs) const;
    
    Vec4i32 operator|(const Vec4i32& rhs) const;
    Vec4i32 operator^(const Vec4i32& rhs) const;
    Vec4i32 operator!() const;
    
    // TODO: Some conviniences later to return 4 bools instead
    Vec4i32 operator==(const Vec4i32& rhs) const;
    Vec4i32 operator>(const Vec4i32& rhs) const;
    Vec4i32 operator<(const Vec4i32& rhs) const;
    Vec4i32 operator>=(const Vec4i32& rhs) const;
    Vec4i32 operator<=(const Vec4i32& rhs) const;
private:
    __m128i _mi;
};

#endif
