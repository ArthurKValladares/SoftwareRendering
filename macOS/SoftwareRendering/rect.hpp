#ifndef rect_hpp
#define rect_hpp

struct Rect2D {
    int minX;
    int minY;
    int maxX;
    int maxY;
};

Rect2D ClipRect(int width, int height, Rect2D rect);
#endif
