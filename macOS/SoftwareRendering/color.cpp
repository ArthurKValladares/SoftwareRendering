#include "color.hpp"

Color mul(Color c, float s) {
    return Color{
        (uint8_t) ((float) c.red * s),
        (uint8_t) ((float) c.green * s),
        (uint8_t) ((float) c.blue * s),
    };
}

Color add(Color l, Color r) {
    return Color{
        (uint8_t) ((int) l.red + r.red),
        (uint8_t) ((int) l.green + r.green),
        (uint8_t) ((int) l.blue + r.blue),
    };
}
