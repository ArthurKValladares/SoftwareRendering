#pragma once

#include "point.h"
#include <optional>

struct Rect2D {
    int minX;
    int minY;
    int maxX;
    int maxY;
};

Rect2D BoundingBox(Point2D p0, Point2D p1, Point2D p2);
Rect2D ClipRect(int width, int height, Rect2D rect);
std::optional<Rect2D> Intersection(Rect2D rect0, Rect2D rect1);

struct Rect3D_f {
    float minX;
    float minY;
    float minZ;
    float maxX;
    float maxY;
    float maxZ;
};