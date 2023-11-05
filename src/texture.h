#ifndef texture_h
#define texture_h

#include "defs.h"
#include "color.hpp"
#include <string>

struct Texture {
    Texture(const char* path);

    Color get_pixel_from_idx(u32 idx) const;
    Color get_pixel_xy(u32 x, u32 y) const;
    Color get_pixel_uv(float u, float v) const;
    
    int m_width, m_height, m_channels;
    int m_stride;
    unsigned char *m_img;
};

#endif
