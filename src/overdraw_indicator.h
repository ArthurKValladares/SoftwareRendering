#pragma once

#include "defs.h"

struct OverdrawBuffer {
    OverdrawBuffer(u32 width, u32 height);
    
    u8 ValueAt(u32 width, u32 height) const;
    u8 Increase(u32 width, u32 height);

    void Clear();

    u32 m_width;
    u32 m_height;
    u8* m_pixels;
};