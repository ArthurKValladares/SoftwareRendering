#pragma once

#include "point.h"
#include "vector.h"

struct Camera {
    Point3D_f point;
    Vec3D_f direction;
};