#include "triangle.h"
#include "defs.h"
#include "SIMD/vec4i32.h"
#include "edge_function.h"

Rect2D ScreenTriangle::bounding_box() const {
    const int minY = MIN3(v0.y, v1.y, v2.y);
    const int minX = MIN3(v0.x, v1.x, v2.x);
    const int maxX = MAX3(v0.x, v1.x, v2.x);
    const int maxY = MAX3(v0.y, v1.y, v2.y);
    return Rect2D{minX, minY, maxX, maxY};
}

ScreenTriangle Triangle::project_to_surface(SDL_Surface *surface, const Viewport& viewport) const {
    return ScreenTriangle {
        viewport.project_to_surface(surface, this->v0.p).round(),
        viewport.project_to_surface(surface, this->v1.p).round(),
        viewport.project_to_surface(surface, this->v2.p).round(),
    };   
}