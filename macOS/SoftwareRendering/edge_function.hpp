#ifndef edge_function_hpp
#define edge_function_hpp

#include "defs.h"
#include "vec4i32.hpp"
#include "point.hpp"

struct EdgeFunction {
    Vec4i32 Init(Point2D v0, Point2D v1, Point2D p);

    Vec4i32 step_size_x;
    Vec4i32 step_size_y;
    
    static const i32 step_increment_y = 1;
    static const i32 step_increment_x = 4;
};

#endif
