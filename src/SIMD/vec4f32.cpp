#include "vec4f32.hpp"

#include <utility>

Vec4f32::Vec4f32() {
    _mf = _mm_setzero_ps();
}
Vec4f32::Vec4f32(float val) {
    _mf = _mm_set1_ps(val);
}
Vec4f32::Vec4f32(float a, float b, float c, float d) {
    _mf = _mm_set_ps(a, b, c, d);
}
Vec4f32::Vec4f32(__m128 a) {
    _mf = std::move(a);
}


float Vec4f32::x() const {
    return _xyzw[0];
}
float Vec4f32::y() const {
    return _xyzw[1];
}
float Vec4f32::z() const {
    return _xyzw[2];
}
float Vec4f32::w() const {
    return _xyzw[3];
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
