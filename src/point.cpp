#include "point.h"
#include "defs.h"

#include <cmath>

Point2D Point2D::clamp(Point2D min, Point2D max) const {
    const int x = CLAMP(this->x, min.x, max.x);
    const int y = CLAMP(this->y, min.y, max.y);
    return Point2D{x,y};
}

Point2D Point2D::operator+(const Point2D& rhs) const {
    return Point2D{x + rhs.x, y + rhs.y};
}

Point2D Point2D::operator-(const Point2D& rhs) const {
    return Point2D{x - rhs.x, y - rhs.y};
}

Point2D rotate_point(Point2D point, Point2D pivot, float angle) {
    const float s = sin(angle);
    const float c = cos(angle);

    const Point2D p = point - pivot;

    const float new_x = p.x * c - p.y * s;
    const float new_y = p.x * s + p.y * c;

    return Point2D{(int) round(new_x) + pivot.x, (int) round(new_y) + pivot.y};
}

Point2D Point2D_f::round() const {
    return Point2D {
        (int) ::round(this->x),
        (int) ::round(this->y)
    };
}

Point3D Point3D_f::round() const {
    return Point3D {
        (int) ::round(this->x),
        (int) ::round(this->y),
        (int) ::round(this->z)
    };
}