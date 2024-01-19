#include "camera.h"

Mat4f32 Camera::GetProjMatrix() const {
    const OrtographicData& d = m_orthographic;

    const float w_inv = 1.0 / (d.x_max - d.x_min);
    const float h_inv = 1.0 / (d.y_max - d.y_min);
    const float d_inv = 1.0 / (d.z_max - d.z_min);

    return Mat4f32(
        Vec4f32(2.0 * w_inv, 0.0,         0.0,   -(d.x_max + d.x_min) * w_inv),
        Vec4f32(0.0,         2.0 * h_inv, 0.0,   -(d.y_max + d.y_min) * h_inv),
        Vec4f32(0.0,         0.0,         d_inv, -d.z_min * d_inv),
        Vec4f32(0.0,         0.0,         1.0,   0.0)
    );
}