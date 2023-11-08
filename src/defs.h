#ifndef defs_h
#define defs_h

#include <stdint.h>
#include <assert.h>

#ifdef __APPLE__
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define MAX3(a, b, c) MAX(a, MAX(b, c))
#define MIN3(a, b, c) MIN(a, MIN(b, c))

#define CLAMP(val, min, max) val > max ? max : val < min ? min : val;

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#endif
