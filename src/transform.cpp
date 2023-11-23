#include "transform.h"

Mat4f32 rotate_matrix(Vec3D_f axis, float angle) {
    const Vec3D_f a = axis.normalized();

    const float s = sin(angle);
    const float c = cos(angle);
    const float d = 1.0 - c;

    const float x = a.x * d;
    const float y = a.y * d;
    const float z = a.z * d;

    const float axay = x * a.y;
    const float axaz = x * a.z;
    const float ayaz = y * a.z;

    return Mat4f32(
        Vec4f32(c + x * a.x,    axay - s * a.z, axaz + s * a.y, 0.0),
        Vec4f32(axay + s * a.z, c + y * a.y,    ayaz - s * a.x, 0.0),
        Vec4f32(axaz - s * a.y, ayaz + s * a.x, c + z * a.z,    0.0),
        Vec4f32(0.0,            0.0,            0.0,            1.0)
    );
}