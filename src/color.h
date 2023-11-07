#ifndef color_h
#define color_h

#include <cstdint>

struct Color {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    
    Color operator*(float s) const;
    Color operator+(const Color& rhs) const;
};

#endif
