#pragma once

#include "point.h"
#include "math/vec3f32.h"
#include "math/mat4f32.h"

struct OrtographicData {
        float x_min;
        float x_max;
        float y_min;
        float y_max;
        float z_min;
        float z_max;
    };

struct Camera {
    Mat4f32 GetProjMatrix() const;

    OrtographicData m_orthographic;
};