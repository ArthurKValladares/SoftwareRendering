#ifndef texture_h
#define texture_h

#include <string>

struct Texture {
    int width, height, channels;
    unsigned char *img;
};

Texture create_texture(const char *path);

#endif
