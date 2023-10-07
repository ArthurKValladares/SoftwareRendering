#ifndef integer_hpp
#define integer_hpp

#include "defs.h"
#include "sse2neon.h"

struct i128 {
    i128(i32 a, i32 b, i32 c, i32 d);
private:
    __m128i _mi;
};

#endif
