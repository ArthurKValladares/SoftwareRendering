#include "vec4f32.hpp"

Vec4f32::Vec4f32() {}

Vec4f32::Vec4f32(float val) {
    _mf = _mm_set1_ps(val);
}

Vec4f32::Vec4f32(float a, float b, float c, float d) {
    _mf = _mm_set_ps(a, b, c, d);
}

Vec4f32::Vec4f32(__m128 a) {
    _mf = a;
}

void Vec4f32::store(float* dest) const {
    _mm_storeu_ps(dest, _mf);
}

Vec4f32 Vec4f32::operator+(const Vec4f32& rhs) const {
    return Vec4f32(_mm_add_ps(_mf, rhs._mf));
}

Vec4f32 Vec4f32::operator*(const Vec4f32& rhs) const {
    return Vec4f32(_mm_mul_ps(_mf, rhs._mf));
}

Vec4f32 Vec4f32::operator/(const Vec4f32& rhs) const {
    return Vec4f32(_mm_div_ps(_mf, rhs._mf));
}
