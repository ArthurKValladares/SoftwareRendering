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
    _mf = _mm_set_ps(d, c, b, a);
}
Vec4f32::Vec4f32(Point3D_f p, float d) {
    _mf = _mm_set_ps(d, p.z, p.y, p.x);
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

Vec4f32 Vec4f32::normalized() const {
    const auto sum = x() + y() + z() + w();
    return Vec4f32(x() / sum, y() / sum, z() / sum, w() / sum);
}
Vec4f32 Vec4f32::min(float val) const {
    return Vec4f32(_mm_min_ps(_mf, _mm_set1_ps(val)));
}
Vec4f32 Vec4f32::max(float val) const {
    return Vec4f32(_mm_max_ps(_mf, _mm_set1_ps(val)));
}
Vec4f32 Vec4f32::clamp(float min, float max) const {
    const auto a_min = _mm_min_ps(_mf, _mm_set1_ps(max));
    return Vec4f32(_mm_max_ps(a_min, _mm_set1_ps(min)));
}
Vec4f32 Vec4f32::modf1() const {
    // Emulating UV sampling where values outside [0.0, 1.0] repeat the sampled texture.
    // In this case, positive values simply use the fractional part of the value for sampling,
    // but negative values use (1.0 - fractional part). This is a slightly optimized way of doing that.
    __m128 integer = _mm_round_ps(_mf, _MM_FROUND_TRUNC);
    __m128 fraction = _mm_sub_ps(_mf, integer);
    fraction = _mm_add_ps(fraction, _mm_set1_ps(1.0));
    integer = _mm_round_ps(fraction, _MM_FROUND_TRUNC);
    return Vec4f32(_mm_sub_ps(fraction, integer));
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
    // TODO: can this be better?
    const Vec4f32 mul = *this * rhs;
    const __m128 hadd = _mm_hadd_ps(mul._mf, mul._mf);
    return Vec4f32(_mm_hadd_ps(hadd, hadd)).x();
}

float Vec4f32::operator[](i32 index) const {
    return _xyzw[index];
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
