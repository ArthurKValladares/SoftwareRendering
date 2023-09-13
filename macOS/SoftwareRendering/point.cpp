#include "point.hpp"
#include <cmath>

Point2D point_sub(Point2D a, Point2D b) {
    return Point2D{a.x - b.x, a.y - b.y};
}

Point2D rotate_point(Point2D point, Point2D pivot, float angle) {
    const float s = sin(angle);
    const float c = cos(angle);

    const Point2D p = point_sub(point, pivot);

    const float new_x = p.x * c - p.y * s;
    const float new_y = p.x * s + p.y * c;

    return Point2D{(int) round(new_x) + pivot.x, (int) round(new_y) + pivot.y};
}
