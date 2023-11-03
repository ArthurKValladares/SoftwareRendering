#include "color.hpp"

Color Color::operator*(float s) const {
    return Color{
        (uint8_t) ((float) red * s),
        (uint8_t) ((float) green * s),
        (uint8_t) ((float) blue * s),
    };
}

Color Color::operator+(const Color& rhs) const {
    return Color{
        (uint8_t) ((int) red +  rhs.red),
        (uint8_t) ((int) green + rhs.green),
        (uint8_t) ((int) blue +  rhs.blue),
    };
}
