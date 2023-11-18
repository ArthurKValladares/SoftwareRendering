#include "math/vec3f32.h"

Vec3D_f Vec3D_f::normalized() const {
    const float sum = x + y + z;
    return Vec3D_f {
        x / sum,
        y / sum,
        z / sum
    };
}