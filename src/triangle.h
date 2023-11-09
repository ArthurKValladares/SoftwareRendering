#ifndef triangle_h
#define triangle_h

#include "point.h"
#include "color.h"
#include "uv.h"
#include "rect.h"
#include "vector.h"
#include "vertex.h"

struct Triangle {
    const Vertex& v0;
    const Vertex& v1;
    const Vertex& v2;
};

#endif
