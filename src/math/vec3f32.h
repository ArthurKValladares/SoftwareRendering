#pragma once

struct Vec2D_f {
    float x;
    float y;
};

struct Vec3D_f {
    float x;
    float y;
    float z;

    Vec3D_f normalized() const;
    float dot(const Vec3D_f& rhs) const;
    Vec3D_f cross(const Vec3D_f& rhs) const;

    Vec3D_f operator+(const Vec3D_f& rhs) const;
    Vec3D_f operator-(const Vec3D_f& rhs) const;
};