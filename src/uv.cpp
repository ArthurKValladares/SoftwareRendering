#include "uv.h"

UV UV::operator*(float s) const {
    return UV{
        u * s,
        v * s,
    };
}
