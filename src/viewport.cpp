#include "viewport.h"

Point2D_f Viewport::viewport_to_surface(SDL_Surface *surface, Point2D_f p) const {
    return Point2D_f{p.x * surface->w / this->width, p.y * surface->h / this->height};
}

Point2D_f Viewport::project_to_viewport(Point3D_f p) const {
    const float d_n = this->distance / p.z;
    return Point2D_f{p.x * d_n, p.y * d_n};
}

Point2D_f Viewport::project_to_surface(SDL_Surface *surface, Point3D_f p) const {
    return this->viewport_to_surface(surface, this->project_to_viewport(p));
}