#include "integer.hpp"

SIMDVec4::SIMDVec4(i32 a, i32 b, i32 c, i32 d) {
    _mi = _mm_set_epi32(a, b, c, d);
}

SIMDVec4::SIMDVec4(__m128i a) {
    _mi = a;
}

SIMDVec4 SIMDVec4::operator+(const SIMDVec4&rhs) {
    return SIMDVec4(_mm_add_epi32(_mi, rhs._mi));
}

SIMDVec4 SIMDVec4::operator-(const SIMDVec4&rhs) {
    return SIMDVec4(_mm_sub_epi32(_mi, rhs._mi));
}

SIMDVec4 SIMDVec4::operator*(const SIMDVec4& rhs) {
    return SIMDVec4(_mm_mullo_epi32(_mi, rhs._mi));
}
