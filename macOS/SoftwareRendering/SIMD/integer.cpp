#include "integer.hpp"

Vec4i32::Vec4i32() {}

Vec4i32::Vec4i32(i32 a, i32 b, i32 c, i32 d) {
    _mi = _mm_set_epi32(a, b, c, d);
}

Vec4i32::Vec4i32(i32 a) {
    _mi = _mm_set1_epi32(a);
}

Vec4i32::Vec4i32(__m128i a) {
    _mi = a;
}

void Vec4i32::store(i32* dest) const {
    _mm_storeu_si128((__m128i*) dest, _mi);
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

void Vec4i32::operator+=(const Vec4i32&rhs) {
    _mi = (*this + rhs)._mi;
}
void Vec4i32::operator-=(const Vec4i32&rhs) {
    _mi = (*this - rhs)._mi;
}
void Vec4i32::operator*=(const Vec4i32& rhs) {
    _mi = (*this * rhs)._mi;
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

bool Vec4i32::operator==(const Vec4i32& rhs) const {
    const __m128i i = _mm_cmpeq_epi32(_mi, rhs._mi);
    return !_mm_testz_si128(i, i);
}
bool Vec4i32::operator>(const Vec4i32& rhs) const {
    const __m128i i = _mm_cmpgt_epi32(_mi, rhs._mi);
    return !_mm_testz_si128(i, i);
}
bool Vec4i32::operator<(const Vec4i32& rhs) const {
    const __m128 i = _mm_cmplt_epi32(_mi, rhs._mi);
    return !_mm_testz_si128(i, i);
}
bool Vec4i32::operator>=(const Vec4i32& rhs) const {
    return !(*this < rhs);
}
bool Vec4i32::operator<=(const Vec4i32& rhs) const {
    return !(*this > rhs);
}
