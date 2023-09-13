#ifndef color_hpp
#define color_hpp

#include <cstdint>

struct Color {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

Color mul(Color c, float s);
Color add(Color l, Color r);

#endif
