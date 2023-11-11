#include "vec4f32.h"
#include "vec4i32.h"

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

Vec4i32 Vec4f32::to_int_nearest() const {
    return Vec4i32(_mm_cvtps_epi32(_mf));
}
Vec4i32 Vec4f32::to_int_round_down() const {
    return Vec4i32(_mm_cvttps_epi32(_mf));
}


Vec4f32 Vec4f32::min(float val) {
    return Vec4f32(_mm_min_ps(_mf, _mm_set1_ps(val)));
}
Vec4f32 Vec4f32::max(float val) {
    return Vec4f32(_mm_max_ps(_mf, _mm_set1_ps(val)));
}
Vec4f32 Vec4f32::clamp(float min, float max) {
    const auto a_min = _mm_min_ps(_mf, _mm_set1_ps(max));
    return Vec4f32(_mm_max_ps(a_min, _mm_set1_ps(min)));
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

float Vec4f32::dot(const Vec4f32& rhs) const {
    const Vec4f32 mul = *this * rhs;
    return mul.x() + mul.y() + mul.z() + mul.w();
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
