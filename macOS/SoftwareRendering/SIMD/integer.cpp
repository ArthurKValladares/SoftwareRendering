#include "integer.hpp"

i128::i128(i32 a, i32 b, i32 c, i32 d) {
    _mi = _mm_set_epi32(a, b, c, d);
}
