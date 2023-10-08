#include "integer.hpp"

Vec4i32::Vec4i32(i32 a, i32 b, i32 c, i32 d) {
    _mi = _mm_set_epi32(a, b, c, d);
}

Vec4i32::Vec4i32(i32 a) {
    _mi = _mm_set_epi32(a, a, a, a);
}

Vec4i32::Vec4i32(__m128i a) {
    _mi = a;
}

Vec4i32 Vec4i32::operator+(const Vec4i32&rhs) const {
    return Vec4i32(_mm_add_epi32(_mi, rhs._mi));
}
Vec4i32 Vec4i32::operator-(const Vec4i32&rhs) const {
    return Vec4i32(_mm_sub_epi32(_mi, rhs._mi));
}
Vec4i32 Vec4i32::operator*(const Vec4i32& rhs) const {
    return Vec4i32(_mm_mullo_epi32(_mi, rhs._mi));
}

Vec4i32 Vec4i32::operator|(const Vec4i32& rhs) const {
    return Vec4i32(_mm_or_si128(_mi, rhs._mi));
}
Vec4i32 Vec4i32::operator^(const Vec4i32& rhs) const {
    return Vec4i32(_mm_xor_si128(_mi, rhs._mi));
}
Vec4i32 Vec4i32::operator!() const {
    return *this ^ Vec4i32(0xffff);
}

Vec4i32 Vec4i32::operator==(const Vec4i32& rhs) const {
    return Vec4i32(_mm_cmpeq_epi32(_mi, rhs._mi));
}
Vec4i32 Vec4i32::operator>(const Vec4i32& rhs) const {
    return Vec4i32(_mm_cmpgt_epi32(_mi, rhs._mi));
}
Vec4i32 Vec4i32::operator<(const Vec4i32& rhs) const {
    return Vec4i32(_mm_cmplt_epi32(_mi, rhs._mi));
}
Vec4i32 Vec4i32::operator>=(const Vec4i32& rhs) const {
    return Vec4i32(_mm_or_si128(_mi, rhs._mi));
}
Vec4i32 Vec4i32::operator<=(const Vec4i32& rhs) const {
    return Vec4i32(_mm_or_si128(_mi, rhs._mi));
}
