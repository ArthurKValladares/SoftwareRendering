#pragma once

#include "defs.h"

struct DepthBuffer {
    DepthBuffer(u32 width, u32 height);
    
    float* pixels;
};