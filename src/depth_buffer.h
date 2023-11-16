#pragma once

#include "defs.h"

struct DepthBuffer {
    DepthBuffer(u32 width, u32 height);
    
    float ValueAt(u32 width, u32 height) const;
    void Write(u32 width, u32 height, float depth);

    void Clear();

    u32 m_width;
    u32 m_height;
    float* m_pixels;
};