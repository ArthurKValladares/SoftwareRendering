#pragma once

#include "point.h"
#include "math/vec3f32.h"
#include "math/mat4f32.h"

enum class CameraType {
    Perspective,
    Ortographic,
};

struct OrtographicCamera {
    float x_min;
    float x_max;
    float y_min;
    float y_max;
    float z_min;
    float z_max;
};

struct PerspectiveCamera {
    Vec3D_f position;
    Vec3D_f front;
    Vec3D_f world_up;
    float y_fov;
    float z_near;
    // TODO: change this when resizing window, if I support it
    // Also take it from surface in a constructor
    float aspect_ratio;
};

struct Camera {
    static Camera orthographic(OrtographicCamera data);
    static Camera perspective(PerspectiveCamera data);

    Mat4f32 GetProjMatrix() const;
    Mat4f32 GetViewMatrix() const;

    CameraType m_ty;
    union {
        OrtographicCamera m_orthographic;
        PerspectiveCamera m_perspective;
    };
};