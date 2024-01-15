#include "camera.h"

namespace {
    Mat4f32 infinite_perspectice_proj(float aspect_ratio, float y_fov, float z_near) {
        const auto f = 1.0 / tan(y_fov / 2.0);
        return Mat4f32(
            Vec4f32(f / aspect_ratio, 0., 0.0, 0.0),
            Vec4f32(0.0,              f,  0.0, 0.0),
            Vec4f32(0.0,              0., 0.0, z_near),
            Vec4f32(0.0,              0., 1.0, 0.0)
        );
    }

    Mat4f32 look_to(Vec3D_f eye, Vec3D_f front, Vec3D_f world_up) {
        const Vec3D_f front = front.normalized();
        const Vec3D_f side = world_up.cross(front).normalized();
        const Vec3D_f up = front.cross(side);

        return Mat4f32(
            Vec4f32(side.x,  side.y,  side.z,  -side.dot(eye)),
            Vec4f32(up.x,    up.y,    up.z,    -up.dot(eye)),
            Vec4f32(front.x, front.y, front.z, -front.dot(eye)),
            Vec4f32(0.0,     0.0,     0.0,      1.0)
        );
    }
}

Camera Camera::orthographic(OrtographicCamera data) {
    return Camera {
        CameraType::Ortographic,
        data
    };
}

Camera Camera::perspective(PerspectiveCamera data) {
    Camera ret = Camera {
        CameraType::Perspective
    };
    ret.m_perspective = std::move(data);
    return ret;
}

Mat4f32 Camera::GetProjMatrix() const {
    switch (this->m_ty) {
        case CameraType::Ortographic: {
            const OrtographicCamera& d = m_orthographic;

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
        case CameraType::Perspective: {
            // TODO
            return Mat4f32::identity();
        }
    }
}