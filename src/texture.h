#ifndef texture_h
#define texture_h

#include "defs.h"
#include "color.h"
#include <string>

struct Texture {
    Texture(const char* path, SDL_Surface *surface);
    void free();

    Uint32 get_pixel_from_idx(u32 idx) const;
    Uint32 get_pixel_xy(u32 x, u32 y) const;
    Uint32 get_pixel_uv(float u, float v) const;
    
    int m_width, m_height;
    Uint32* m_img;
};

#endif
