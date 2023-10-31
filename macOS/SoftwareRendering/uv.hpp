#pragma once

#include "defs.h"

struct UV {
    float u;
    float v;
    
    UV operator*(float s) const;
};

