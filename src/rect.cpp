#include "rect.h"
#include "defs.h"

Rect2D ClipRect(int width, int height, Rect2D rect) {
    const int minX = MAX(rect.minX, 0);
    const int minY = MAX(rect.minY, 0);
    const int maxX = MIN(rect.maxX, width - 1);
    const int maxY = MIN(rect.maxY, height - 1);
    return Rect2D{minX, minY, maxX, maxY};
}
