#include "math/vec3f32.h"

Vec3D_f Vec3D_f::normalized() const {
    const float sum = x + y + z;
    return Vec3D_f {
        x / sum,
        y / sum,
        z / sum
    };
}

float Vec3D_f::dot(const Vec3D_f& rhs) const {
    return x * rhs.x + y * rhs.y + z*rhs.z;
}

Vec3D_f Vec3D_f::cross(const Vec3D_f& rhs) const {
    return Vec3D_f {
        y * rhs.z - z * rhs.y,
        z * rhs.x - x * rhs.z,
        x * rhs.y - y * rhs.x,
    };
}