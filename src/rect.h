#pragma once

#include <optional>

struct Rect2D {
    int minX;
    int minY;
    int maxX;
    int maxY;
};

Rect2D ClipRect(int width, int height, Rect2D rect);
std::optional<Rect2D> Intersection(Rect2D rect0, Rect2D rect1);