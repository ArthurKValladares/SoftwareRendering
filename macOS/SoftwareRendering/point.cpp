#include "point.hpp"
#include <cmath>

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
