#pragma once

#include "math/mat4f32.h"
#include "math/vec3f32.h"

Mat4f32 rotate_matrix(Vec3D_f axis, float angle);
Mat4f32 uniform_scale_matrix(float scale);
Mat4f32 translation_matrix(float x, float y, float z);