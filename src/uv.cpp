#include "uv.hpp"

UV UV::operator*(float s) const {
    return UV{
        u * s,
        v * s,
    };
}
