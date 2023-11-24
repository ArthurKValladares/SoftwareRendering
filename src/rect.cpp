#include "rect.h"
#include "defs.h"

Rect2D ClipRect(int width, int height, Rect2D rect) {
    const int minX = MAX(rect.minX, 0);
    const int minY = MAX(rect.minY, 0);
    const int maxX = MIN(rect.maxX, width - 1);
    const int maxY = MIN(rect.maxY, height - 1);
    return Rect2D{minX, minY, maxX, maxY};
}

std::optional<Rect2D> Intersection(Rect2D rect0, Rect2D rect1) {
    const int minX = MAX(rect0.minX, rect1.minX);
    const int maxX = MIN(rect0.maxX, rect1.maxX);
    const int minY = MAX(rect0.minY, rect1.minY);
    const int maxY = MIN(rect0.maxY, rect1.maxY);

    if (minX >= maxX ||  minY >= maxY) {
        return std::nullopt;
    } else {
        return Rect2D{
            minX,
            minY,
            maxX,
            maxY
        };
    }
}