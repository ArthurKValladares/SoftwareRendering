#include "depth_buffer.h"

DepthBuffer::DepthBuffer(u32 width, u32 height) {
    pixels = (float*) malloc(width * height * sizeof(float));
}