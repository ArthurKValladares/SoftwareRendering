#include "camera.h"

namespace {
    Mat4f32 infinite_perspectice_proj(float aspect_ratio, float y_fov, float z_near) {
        const auto far = 1000.0;

        const auto g = 1.0 / tan(y_fov * 0.5); 
        const auto k = far / (far - z_near);

        return Mat4f32(
            Vec4f32(g / aspect_ratio, 0., 0.0, 0.0),
            Vec4f32(0.0,              g,  0.0, 0.0),
            Vec4f32(0.0,              0., k, -z_near * k),
            Vec4f32(0.0,              0., 1.0, 0.0)
        );
    }

    Mat4f32 look_to(Vec3D_f eye, Vec3D_f front, Vec3D_f world_up) {
        const Vec3D_f forward = front.normalized();
        const Vec3D_f right   = world_up.cross(forward).normalized();
        const Vec3D_f up      = forward.cross(right).normalized();

        const float translation_x = eye.dot(right);
        const float translation_y = eye.dot(up);
        const float translation_z = eye.dot(forward);

        return Mat4f32(
            Vec4f32(right.x,        up.x,           forward.x,      0.0),
            Vec4f32(right.y,        up.y,           forward.y,      0.0),
            Vec4f32(right.z,        up.z,           forward.z,      0.0),
            Vec4f32(-translation_x, -translation_y, -translation_z, 1.0)
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
            return infinite_perspectice_proj(m_perspective.aspect_ratio, m_perspective.y_fov, m_perspective.z_near);
        }
    }
}

Mat4f32 Camera::GetViewMatrix() const {
    switch (this->m_ty) {
        case CameraType::Ortographic: {
            return Mat4f32::identity();
        }
        case CameraType::Perspective: {
            return look_to(m_perspective.position, m_perspective.front, m_perspective.world_up);
        }
    }
}